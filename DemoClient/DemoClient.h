#ifndef DEMO_DEMOCLIENT_H
#define DEMO_DEMOCLIENT_H

#include <cstdint>
#include "../Common/TCPSocket.h"
#include "../Common/base.h"
#include "../Common/MesHead.h"
#include "../Common/Epoll.h"

struct ClientInfo
{
    std::string ip;
    int port;
    int bufferlen;
};

struct ServerInfo
{
    std::string ip;
    int port;
};
class DemoClient
{
private:
    DemoClient(const DemoClient &demoClient);
    DemoClient &operator=(const DemoClient &demoClient);

public:
    DemoClient(ClientInfo tmp, ServerInfo ser);
    ~DemoClient(){};

    int working();

    int send_msg(int uin, int cmd_id, google::protobuf::Message &msg);

    int process_code(const char *pszInCode, const int iInCodeSize, int socketfd);

    int solve_code(char *body, const int id);

    int set_send_playerinfo(int funcId, int playerId, std::string mesg);

    int recv_mesg();

    void handle_input_and_send();

private:
    std::shared_ptr<TCPSocket> m_client_conn_socket;
    std::shared_ptr<TCPSocket> m_client_read_socket;
    ClientInfo m_clientinfo;
    ServerInfo m_serverinfo;
    char m_recv_buf[MAX_SS_PACKAGE_SIZE];
    Epoll m_epoll;
};
#endif