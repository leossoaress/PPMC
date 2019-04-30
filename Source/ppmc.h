#ifndef PPMC_PPMC_H_
#define PPMC_PPMC_H_

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "Node.h"

class PPMC {

private:
    
    unsigned int numCh = 256;
    unsigned int k = 0;
    
    bool usedCh[256] = {false};    
    int order = 0;
    
    std::vector<Node*> excluded;

    Node* root = nullptr;
    Node* base = nullptr;
    
    std::fstream input;

    void Encode(unsigned char symbol_);

    void InsertSymbol(unsigned char symbol_);

    void GetInterval(Node *pDad, Node *pChild, bool *arr);

    void GetInverval2(unsigned char symbol_);

    int GetContextDepth(Node *node);

public:

    PPMC(unsigned int order_, std::string input_);

    void Compress();

    void PrintTree(Node* node, int deep);
    
};

#endif //PPMC_PPMC_H_