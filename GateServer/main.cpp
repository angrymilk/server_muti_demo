#include "GateServer.h"

int main(int argc, char **argv)
{
    std::shared_ptr<GateServer> ptr(new GateServer());
    ptr->run();
    return 0;
}