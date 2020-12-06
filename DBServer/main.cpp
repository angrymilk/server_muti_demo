#include "DBServer.h"

int main(int argc, char **argv)
{
    std::shared_ptr<GameServer> ptr(new GameServer());
    ptr->run();
    return 0;
}