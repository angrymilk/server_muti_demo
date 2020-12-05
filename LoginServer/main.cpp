#include "LoginServer.h"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("IP PORT INFO must be input\n");
        return 0;
    }
    std::shared_ptr<LoginServer> ptr(new LoginServer());
    ptr->run();
    return 0;
}