#include "ppmc.h"

int main(int argc, char const *argv[])
{
    PPMC ppm(0, "msg.txt");

    ppm.Compress();

    return 0;
}
