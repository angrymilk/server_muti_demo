#include "GameServer.h"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("IP PORT INFO must be input\n");
        return 0;
    }
    std::shared_ptr<GameServer> ptr(new GameServer(string(argv[1]), atoi(argv[2])));
    ptr->run();
    return 0;
}