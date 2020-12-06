#ifndef DEMO_SERVER_H
#define DEMO_SERVER_H
#define MYSQLPP_SSQLS_NO_STATICS
#include "../Common/BaseServer.h"
#include "../Common/MesHead.h"
#include "Player.h"
#include "../Common/TCPSocket.h"
#include "../Common/ThreadTask.h"
#include "../Common/RedisServer.h"
#include "../Common/SQLServer.h"
struct PlayerInfo
{
    int fd;
    shared_ptr<Player> player;
};

class DBServer
{
public:
    DBServer();
    ~DBServer()
    {
    }
    int run();
    int on_message(TCPSocket &con);
    void get_one_code(TCPSocket &con);
    void send(char *data, int size);
    void transmit(TCPSocket &con, std::string &data, int datasize);
    bool check_user(int uid);

private:
    std::shared_ptr<BaseServer> m_server;
    std::vector<int> m_con;
    std::unordered_map<int, int> m_player_con;
};
#endif