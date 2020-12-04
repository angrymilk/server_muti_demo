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
    GameServer(std::string ip, int port);
    ~GameServer()
    {
    }
    int run();
    int on_message(TCPSocket &con);
    void get_one_code(TCPSocket &con);
    void solve(TCPSocket &con, std::string &data, int datasize);
    void serialize(TCPSocket &con, std::string &data, std::string &out, int type);
    void parse(char *input, int &size, int type);
    void send(char *data, int size);
    void register(TCPSocket &con, std::string &data, int datasize);

private:
    struct IpPort
    {
        int port;
        string ip;
    };
    IpPort m_send_info;
    std::shared_ptr<BaseServer> m_server;
    std::unordered_map<int, PlayerInfo> m_map_players;
    std::unordered_map<string, int> m_uid_map;
    std::vector<IpPort> db_server_info;
    std::vector<std::shared_ptr<TCPSocket>> m_clients;
    int db_server_num;
};
#endif