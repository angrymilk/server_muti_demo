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

struct SendHelp
{
    char buf[1000];
    int datasize;
};

class GameServer
{
public:
    GameServer();
    ~GameServer()
    {
        m_thread_task.stop();
    }
    int run();
    int on_message(TCPSocket &con);
    void run_function(TCPSocket &con, string &out, int data_size);
    void solve(TCPSocket &con, std::string &data, int datasize);
    void send(SendHelp data, int uid);
    void send_db(SendHelp data, int uid);
    void move_calculate(TCPSocket &con, std::string &data, int datasize);

    //转发gateserver发来的db增删请求
    void solve_add(TCPSocket &con, std::string &data, int datasize);

    //转发dbserver发来的数据查询信息
    void transmit_db(TCPSocket &con, std::string &data, int datasize);

    //转发gateserver发来的db查询请求
    void solve_query(TCPSocket &con, std::string &data, int datasize);
    void parse(char *input, Player &player, int &size);
    void handle_move(Player &player);
    void make_fd(int uid, int fd);
    void serialize(char *data, Player &temp, int &size);
    ThreadTask m_thread_task;

private:
    std::shared_ptr<BaseServer> m_server;
    std::unordered_map<int, PlayerInfo> m_map_players; // hash fd to gateserver
    std::unordered_map<int, int> m_player_db;          //hash fd to dbservers
    std::vector<int> m_db;
    int db_num;
};
#endif