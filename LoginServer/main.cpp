#include "LoginServer.h"

int main(int argc, char **argv)
{
    std::shared_ptr<LoginServer> ptr(new LoginServer());
    ptr->run();
    return 0;
}