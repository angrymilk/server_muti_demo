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
    void solve_add(TCPSocket &con, std::string &data, int datasize);
    void solve_query(TCPSocket &con, std::string &data, int datasize);
    void send(char *data, int size, int uid);

private:
    std::shared_ptr<BaseServer> m_server;
    std::vector<int> m_con;
    std::unordered_map<int, int> m_player_con;
    std::shared_ptr<SQLServer> m_sql_server;
};
#endif