#include "DBServer.h"

int main(int argc, char **argv)
{
    std::shared_ptr<DBServer> ptr(new DBServer());
    ptr->run();
    return 0;
}