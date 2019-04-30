#include "ppmc.h"

PPMC::PPMC(unsigned int order_, std::string inputname, std::string outputname) : order(order_)
{
    root = new Node(ROOT);
    base = root;

    input.open(inputname, std::ios::in | std::ios::binary);
    if(!input) {
        std::cout << "Could not open input file" << std::endl;
        exit(-1);
    }

    input.seekg(0, std::ios::end);
    long size = input.tellg();
    input.seekg(0, std::ios::beg);

    std::vector<unsigned char> v (size/sizeof(char));
    input.read((char *) &v[0], size);

    this->data = new std::string(v.begin(), v.end());
    step_progess = 1.0 / this->data->size();

    output.open(outputname, std::ios::out | std::ios::binary);
    if(!output) {
        std::cout << "Unable to open output file" << std::endl;
        exit(0);
    }

    arithmetic.SetFile(&output);
}

void PPMC::Compress()
{
    clock_t start = std::clock();

    for(auto &ch : *data) {
        Encode(ch);
        InsertSymbol(ch);
        Progress();
    }

    arithmetic.EncodeFinish();
    duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;

    std::cout << std::setprecision(3) << std::fixed;
    std::clog << "\nCompression duration time: " << duration << " segundos" << std::endl;
}

void PPMC::Encode(unsigned char symbol_)
{
    Node* pVine = base->GetVine();
    Node *pActualNode = nullptr;
    bool excluded[256] = {false};

    while(pVine != nullptr) {

        pActualNode = pVine->FindChild(symbol_);

        if(pActualNode == nullptr && pVine->GetChild() != nullptr) {
            //std::cout << "esc: ";
            GetInterval(pVine, pVine->GetEsc(), excluded);

            Node* pAux = pVine->GetChild();
            while(pAux != nullptr) {
                excluded[pAux->GetSymbol()] = true;
                pAux = pAux->GetSibiling();
            }
            
            pVine = pVine->GetVine();
        }
        else if(pActualNode != nullptr){
            //std::cout << pActualNode->GetSymbol() << ": ";
            GetInterval(pVine, pActualNode, excluded);
            return;
        } 
        else {
            pVine = pVine->GetVine();
        }
    }

    if(pVine == nullptr) {
        usedCh[symbol_] = true;
        //std::cout << "Root: ";
        GetInverval2(symbol_);
        return;
    }

}

void PPMC::InsertSymbol(unsigned char symbol_)
{
    int depth = GetContextDepth(base);

    Node *pAuxNode = (depth < order) ? base : base->GetVine();;
    
    Node *pActualNode = pAuxNode->InsertChild(symbol_);
    pActualNode->SetVine(pAuxNode);

    base = pActualNode;

    Node* z = pAuxNode->GetVine();
    Node* b = z;
    Node* a = nullptr;

    int count = 0;

    while(z != nullptr) {
        
        Node *pNode = z->InsertChild(symbol_);
        pNode->SetVine(z);

        b = pNode;

        if(count == 0) {
            base->SetVine(b);
        }
        else {
            a->SetVine(b);
            a = b;
        }
        a = b;
        z = z->GetVine();
        count++;
    }
}

void PPMC::GetInterval(Node *pDad, Node *pChild, bool *arr)
{
    unsigned int sum = 0, sum2 = 0;
    Node* pAuxNode = pDad->GetChild();

    while(pAuxNode != nullptr && pAuxNode != pChild) {
        if(arr[pAuxNode->GetSymbol()] == false) {
            sum += pAuxNode->GetCount();
        }
        else {
            sum2 += pAuxNode->GetCount();
        } 
        pAuxNode = pAuxNode->GetSibiling();
    }

    //printf("(%d,%d,%d)\n", sum, sum+pChild->GetCount(), pDad->GetChildrenCount()-sum2);
    arithmetic.Encode(sum, sum+pChild->GetCount(), pDad->GetChildrenCount()-sum2);
}

void PPMC::GetInverval2(unsigned char symbol_)
{
    unsigned int sum = 0;

    for(unsigned i = 0; i < symbol_; ++i) {
        if(usedCh[i] == false) {
            sum++;
        }
    }

    //printf("(%d,%d,%d)\n", sum, sum+1, numCh);
    arithmetic.Encode(sum, sum+1, numCh);
    numCh--;
}

int PPMC::GetContextDepth(Node *node)
{
    int counter = -1;

    while(node->GetVine() != nullptr) {
        counter++;
        node = node->GetVine();
    }

    return counter;
}

void PPMC::PrintTree(Node* node, int deep)
{
    for(auto i = 0; i < deep; ++i)
        std::cout << "\t";
    
    switch (node->GetType()) {
        case ROOT:
        printf("root\n");
        break;
        
        case ESC: 
        printf("esc(%d)\n", node->GetCount());
        break;
        
        default:
        printf("%c(%d)\n", node->GetSymbol(), node->GetCount());
        break;
    }
    
    if(node->GetChild() != nullptr) {
        PrintTree(node->GetChild(), ++deep);

        if(node->GetEsc() != nullptr) {
            PrintTree(node->GetEsc(), deep);
        }

        if(node->GetSibiling() != nullptr) {
            PrintTree(node->GetSibiling(), --deep);
        }
    }    
}

void PPMC::Progress()
{
    progess = progess + step_progess;

    std::stringstream progress_stream;
    progress_stream << "\rProgress .........................: "
                    << std::fixed << std::setw( 6 )
                    << std::setprecision( 2 )
                    << 100 * progess
                    << "%";

    std::clog << progress_stream.str();
}