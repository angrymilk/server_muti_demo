#ifndef DEMO_SERVER_H
#define DEMO_SERVER_H
#define MYSQLPP_SSQLS_NO_STATICS
#include "../Common/BaseServer.h"
#include "../Common/MesHead.h"
#include "../Common/TCPSocket.h"
#include "../Common/ThreadTask.h"
#include "../Common/RedisServer.h"
#include "../Common/SQLServer.h"
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
    void send(TCPSocket &con, char *data, int size, int uid);
    void transmit(TCPSocket &con, std::string &data, int datasize);
    bool check_user(int uid);
    void board_send(TCPSocket &con, char *data, int size);

private:
    std::shared_ptr<BaseServer> m_server;
    std::vector<int> m_con;
    std::unordered_map<int, int> m_player_con;
    std::unordered_map<int, int> m_con_client;
    std::unordered_map<int, bool> m_client_check;
    std::unordered_map<int, string> m_user_pass;
};
#endif