#include "DemoClient.h"

int main(int argc, char **argv)
{
    if (argc < 5)
    {
        printf("Input:srcip srcport dstip dstport\n");
        return 0;
    }
    ClientInfo cInfo = {string(argv[1]), atoi(argv[2]), 10024};
    ServerInfo sInfo = {string(argv[3]), atoi(argv[4])};
    shared_ptr<DemoClient> ptr(new DemoClient(cInfo, sInfo));
    ptr->working();
    return 0;
}