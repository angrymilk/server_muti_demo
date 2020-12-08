#include "DemoClient.h"
#include <thread>

DemoClient::DemoClient(ClientInfo tmp, ServerInfo ser) : m_clientinfo(tmp), m_serverinfo(ser)
{
    printf("Demo Client Is Starting Now !!!!!!!!!\n");
}

int DemoClient::working()
{
    int ret = 0;
    m_client_conn_socket = make_shared<TCPSocket>();
    ret = m_client_conn_socket->open_as_client(const_cast<char *>(m_clientinfo.ip.c_str()), m_clientinfo.port, m_clientinfo.bufferlen);
    if (ret)
        return ret;

    int tmpIp;
    ret = ip_string_to_addr(const_cast<char *>(m_serverinfo.ip.c_str()), tmpIp);
    if (ret)
        return ret;

    ret = m_client_conn_socket->connect_to(tmpIp, m_serverinfo.port, true, 100000000);
    if (ret)
        return ret;
    cout << "CLient FD\n";

    if (m_epoll.epoll_init(MAX_SOCKET_COUNT) < 0)
    {
        printf("epoll_init failed\n");
        return fail;
    }

    if (m_epoll.epoll_add(m_client_conn_socket->get_fd()) < 0)
    {
        printf("m_epoll.epoll_add fd:%d failed\n", m_client_conn_socket->get_fd());
        return fail;
    }

    if ((ret = m_epoll.epoll_add(STDIN_FILENO)) < 0)
    {
        printf("m_epoll.epoll_add fd:%d failed ret = %d  errno=%d\n", STDIN_FILENO, ret, errno);
        return fail;
    }

    m_client_read_socket.reset(new TCPSocket(STDIN_FILENO));

    cout << "Begin To Run Epoll...................\n";
    for (;;)
    {
        int timeout = -1;
        int fd_count = m_epoll.epoll_wait(timeout);
        cout << "Epoll Wait Ended................\n";

        for (int i = 0; i < fd_count; i++)
        {
            cout << "Epoll Has Happended\n";
            struct epoll_event *pstEvent = m_epoll.get_event(i);
            int socketfd = pstEvent->data.fd;
            if (socketfd == m_client_read_socket->get_fd())
            {
                handle_input_and_send();
            }
            else if (socketfd == m_client_conn_socket->get_fd())
            {
                int ret = m_client_conn_socket->process_data(std::bind(&DemoClient::process_code, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                if (fail == ret)
                {
                    printf("socket process_data failed fd:%d\n", m_client_conn_socket->get_fd());
                    return ret;
                }
            }
            else
            {
                printf("error fd_type\n");
                return -1;
                break;
            }
        }
    }
    return 0;
}

void DemoClient::handle_input_and_send()
{
    char buffer[COMMON_BUFFER_SIZE], data[MAX_SS_PACKAGE_SIZE];
    MsgHead head;
    int len = 0;
    printf("#####################  请描述你的操作: 1:登录操作  2:查询人物拥有的物品 0:注册人物的信息   #######################\n");
    int ret = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (atoi(buffer) == 1)
    {
        RegisterMessageOn res;
        res.set_password("123456");
        res.set_username("chenzun");

        head.m_message_len = MESSAGE_HEAD_SIZE + res.ByteSize();
        len = MESSAGE_HEAD_SIZE + res.ByteSize();
        int codeLength = 0;
        head.encode(data, codeLength);
        res.SerializePartialToArray(data + MESSAGE_HEAD_SIZE, res.ByteSize());
    }
    else if (atoi(buffer) == 2)
    {
    }

    int fd = m_client_conn_socket->get_fd();
    ret = m_client_conn_socket->send_data(data, (size_t)len);
    if (ret < success)
    {
        printf("Send Failed...........\n");
    }
    if (ret > success)
    {
        printf("Send %d Times Until finished......\n", ret);
    }
    printf("send msg  fd:%d    msglen = %d\n", fd, head.m_message_len);
    return;
}

int DemoClient::process_code(const char *pszInCode, const int iInCodeSize, int socketfd)
{
    if (!pszInCode || iInCodeSize <= 0)
    {
        return fail;
    }

    MsgHead stHead;
    if (success != stHead.decode(pszInCode, iInCodeSize))
    {
        printf("[Player::%s]head decode error\n", __FUNCTION__);
        return fail;
    }

    if (stHead.m_message_len != iInCodeSize)
    {
        printf("[Player::%s]the package size decoded from service was not equal with received size.\n", __FUNCTION__);
        return fail;
    }
    int len = stHead.m_message_len & ((1 << 20) - 1);
    int type = (stHead.m_message_len & ((1 << 20) | (1 << 21) | (1 << 22))) >> 20;
    int iBodySize = len - 4;
    if (type == 1)
    {
    }
    //Response res;
    //res.ParseFromArray(pszInCode + MESSAGE_HEAD_SIZE, iBodySize);
    //cout << "Client Get Message From Server Player:[" << res.uid() << "]  Ack:[" << res.ack() << "]\n";
    return 0;
}