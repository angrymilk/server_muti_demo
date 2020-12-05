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

class LoginServer
{
public:
    LoginServer();
    ~LoginServer()
    {
    }
    int run();
    int on_message(TCPSocket &con);
    void get_one_code(TCPSocket &con);
    void send_gate(char *data, int size, int uid);
    void send_client(char *data, int size, int uid);
    void register(TCPSocket &con, std::string &data, int datasize);

private:
    std::shared_ptr<BaseServer> m_server;
    std::unordered_map<int, int> m_fd_map;
    std::unordered_map<int, int> m_con_map;
    std::vector<int> m_con;
    int db_server_num;
};
#endif