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

class GateServer
{
public:
    GateServer();
    ~GateServer()
    {
    }
    int run();
    int on_message(TCPSocket &con);
    void get_one_code(TCPSocket &con);
    void send(char *data, int size, int uid);
    void transmit(TCPSocket &con, std::string &data, int datasize);
    bool check_user(int uid);

private:
    std::shared_ptr<BaseServer> m_server;
    std::vector<int> m_con;
    std::unordered_map<int, int> m_player_con;
    std::unordered_map<int, int> m_con_client;
    std::unordered_map<int, bool> m_client_check;
    std::unordered_map<int, string> m_user_pass;
};
#endif