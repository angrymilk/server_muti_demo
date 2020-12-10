#ifndef DEMO_SERVER_H
#define DEMO_SERVER_H
#define MYSQLPP_SSQLS_NO_STATICS
#include "../Common/BaseServer.h"
#include "../Common/MesHead.h"
#include "../Common/TCPSocket.h"
#include "../Common/ThreadTask.h"
#include "../Common/RedisServer.h"
#include "../Common/SQLServer.h"
#include "../ThirdPart/loadconfig.h"
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
    void register_(TCPSocket &con, std::string &data, int datasize);

private:
    std::shared_ptr<BaseServer> m_server;
    std::unordered_map<int, int> m_fd_map;  //用来记录uid对应的client到loginserver的fd
    std::unordered_map<int, int> m_con_map; //用来记录uid对应的client到gateserver的fd
    std::vector<int> m_con;                 //可供连接使用的gateserver的fd
    std::shared_ptr<SQLServer> m_sql_server;
    int m_gateserver_num;
};
#endif