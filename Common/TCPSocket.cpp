#include "TCPSocket.h"
#include "BaseServer.h"
#include <netinet/tcp.h>
#include <cstring>
#include <cstdio>
using namespace std;

TCPSocket::TCPSocket(BaseServer *server)
{
    m_buffer = make_shared<Buffer>();
    m_server = server;
}

TCPSocket::TCPSocket(int fd, BaseServer *server, ReadFunctor rf)
{
    m_buffer = make_shared<Buffer>();
    m_server = server;
    m_fd = fd;
    m_read_callback = rf;
}

TCPSocket::~TCPSocket()
{
}

int TCPSocket::get_fd()
{
    return m_fd;
}

void TCPSocket::close_socket()
{
    if (m_fd < 0)
    {
        return;
    }
    close(m_fd);
    m_fd = -1;
}

int TCPSocket::recv_data()
{
    int ret = 0;
    if (m_fd < 0)
    {
        return (int)enmConnErrorCode_invalid_socket;
    }

    m_buffer->reset();
    int received_byte = 0;

    do
    {
        ret = m_buffer->copy_front();

        if (ret)
            break;
        //TODO:这里将Buffer内部的指针传出来有风险，假如在recv的时候m_buffer被析构，但是接受的指针来自于m_buffer,会出问题
        received_byte = recv(m_fd, &(m_buffer->get_tail_point()), size_t(m_buffer->get_recv_buffer_size() - m_buffer->get_tail()), 0);
        if (received_byte > 0)
        {
            m_buffer->get_tail() += received_byte;
        }
        else if (0 == received_byte)
        {
            close_socket();
            ret = enmConnErrorCode_peer_closed;
            break;
        }
        else if (EAGAIN != errno)
        {
            printf("errno:%d\n", errno);
            close_socket();
            ret = enmConnErrorCode_unknow;
            break;
        }
    } while (received_byte > 0);
    printf("[Common][TCPSocket.cpp:%d][INFO]:End Recv_Data\n", __LINE__);
    return ret;
}

int TCPSocket::send_data(char *data, size_t size)
{
    int ret = 0;
    if (NULL == data || 0 == size)
    {
        return (int)enmConnErrorCode_invalid_param;
    }

    if (m_fd < 0)
    {
        return (int)enmConnErrorCode_invalid_socket;
    }

    int remainded = size;
    int sended = 0;
    int nTime = 0;

    char *pszTmp = data;

    while (remainded > 0)
    {
        sended = ::send(m_fd, pszTmp, (size_t)remainded, 0);
        if (sended > 0)
        {
            pszTmp += sended;
            remainded -= sended;
        }
        else
        {
            if (sended == 0 || (sended < 0 && EAGAIN != errno && EINTR != errno))
            {
                close_socket();
                ret = (int)enmConnErrorCode_unknow;
                if (sended == 0)
                    ret = enmConnErrorCode_peer_closed;
                break;
            }
        }
        ++nTime;
    }

    if (nTime > 1 && remainded == 0)
    {
        return nTime; //mean "try multi"
    }
    return ret;
}

int TCPSocket::open_as_server(uint16_t port, char *ip)
{
    printf("[Common][TCPSocket.cpp:%d][INFO]:Init Port:%d  IP:%s\n", __LINE__, port, ip);
    if (m_fd < 0)
    {
        close_socket();
    }
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd < 0)
    {
        printf("[Common][TCPSocket.cpp:%d][ERROR]:open socket failed, m_fd < 0, with code[%d]\n", __LINE__, errno);
        m_fd = -1;
        return -1;
    }
    int flags = 1;
    struct linger ling = {0, 0};
    setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags));
    setsockopt(m_fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));

    setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof(flags)); //set TCP_CORK

    if (0 != setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &flags, (int)sizeof(flags)))
    {
        printf("[Common][TCPSocket.cpp:%d][ERROR]:setsockopt failed with code[%d]\n", __LINE__, errno);
        close_socket();
        return -1;
    }

    struct sockaddr_in stSocketAddr;
    memset(&stSocketAddr, 0x0, sizeof(stSocketAddr));
    stSocketAddr.sin_family = AF_INET;
    if (NULL != ip)
    {
        stSocketAddr.sin_addr.s_addr = inet_addr(ip);
    }
    else
    {
        stSocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    stSocketAddr.sin_port = (u_short)htons(port);
    socklen_t addrSize = socklen_t(sizeof(stSocketAddr));
    if (0 != bind(m_fd, (const sockaddr *)&stSocketAddr, addrSize))
    {
        printf("[Common][TCPSocket.cpp:%d][ERROR]:failed with code[%d]\n", __LINE__, errno);
        close_socket();
        return -1;
    }

    if (0 != listen(m_fd, 128))
    {
        printf("[Common][TCPSocket.cpp:%d][ERROR]:listen failed with code[%d]\n", __LINE__, errno);
        close_socket();
        return -1;
    }

    //设置为非阻塞
    set_non_block(m_fd);
    return 0;
}

int TCPSocket::open_as_client(char *localIP, uint16_t localPort, int buffLen)
{
    //open socket
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd < 0)
    {
        m_fd = -1;
        return -1;
    }

    struct sockaddr_in stLocalAddress;
    memset(&stLocalAddress, 0x0, sizeof(stLocalAddress));

    stLocalAddress.sin_family = AF_INET;
    if (NULL != localIP)
    {
        stLocalAddress.sin_addr.s_addr = inet_addr(localIP);
    }
    else
    {
        stLocalAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    if (0 != localPort)
    {
        stLocalAddress.sin_port = htons(localPort);
    }
    int iOptValue = 1;
    if (0 != setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &iOptValue, sizeof(iOptValue)))
    {
        close_socket();
        return -1;
    }
    setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &iOptValue, sizeof(iOptValue)); //set TCP_CORK

    if (buffLen != -1)
    {
        if (0 != setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, (const void *)&buffLen, sizeof(buffLen)))
        {
            return -1;
        }

        if (0 != setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (const void *)&buffLen, sizeof(buffLen)))
        {
            return -1;
        }
    }

    socklen_t addressSize = (socklen_t)sizeof(stLocalAddress);
    if (0 != bind(m_fd, (const struct sockaddr *)&stLocalAddress, addressSize))
    {
        close_socket();
        return -1;
    }
    return 0;
}

int TCPSocket::connect_to(u_long ip, uint16_t port, bool nonblock, int msecond)
{
    if (0 > m_fd)
    {
        return -1;
    }

    struct sockaddr_in stDstAddress;
    memset(&stDstAddress, 0x0, sizeof(stDstAddress));
    stDstAddress.sin_family = AF_INET;
    stDstAddress.sin_addr.s_addr = ip;
    stDstAddress.sin_port = htons(port);
    socklen_t sockSize = (socklen_t)sizeof(stDstAddress);
    if (0 != connect_nonblock(m_fd, &stDstAddress, sockSize, msecond))
    {
        close_socket();
        printf("[Common][TCPSocket.cpp:%d][ERROR]:Client Connect Error Here !\n", __LINE__);
        return -1;
    }
    if (nonblock)
    {
        set_non_block(m_fd);
    }
    return 0;
}

void TCPSocket::set_socket_fd(int fd)
{
    m_fd = fd;
}

int TCPSocket::accept_fd(int fd)
{
    int accepted_sockfd = -1;
    struct sockaddr_in stSocketAddress;
    socklen_t sockaddress_len = (socklen_t)sizeof(stSocketAddress);
    accepted_sockfd = accept(fd, (struct sockaddr *)&stSocketAddress, &sockaddress_len);
    if (0 >= accepted_sockfd)
    {
        printf("[Common][TCPSocket.cpp:%d][ERROR]:accept client(%s:%d) error code = %d, msg = %s\n",
               __LINE__,
               inet_ntoa(stSocketAddress.sin_addr), stSocketAddress.sin_port,
               errno, strerror(errno));
        return -1;
    }

    //设置为非阻塞socket
    int flags = 1;
    if (ioctl(accepted_sockfd, FIONBIO, &flags) && ((flags = fcntl(accepted_sockfd, F_GETFL, 0)) < 0 || fcntl(accepted_sockfd, F_SETFL, flags | O_NONBLOCK) < 0))
    {
        printf("[Common][TCPSocket.cpp:%d][ERROR]:set accepted socket O_NONBLOCK failed, just close it!\n", __LINE__);
        close(accepted_sockfd);
        return -1;
    }

    //开启TCP_NODELAY
    int on = 1;
    setsockopt(accepted_sockfd, IPPROTO_TCP, TCP_NODELAY, (const void *)&on, sizeof(on));
    return accepted_sockfd;
}

int TCPSocket::process_data()
{
    printf("[Common][TCPSocket.cpp:%d][INFO]:Begin To Recv_Data\n", __LINE__);
    int ret = recv_data();
    if (ret < 0)
    {
        if (ret == -3)
            printf("[Common][TCPSocket.cpp:%d][WARNING]:recv_data failed fd:%d errorcode:%d\n", __LINE__, m_fd, ret);
        else
            printf("[Common][TCPSocket.cpp:%d][ERROR]:recv_data failed fd:%d errorcode:%d\n", __LINE__, m_fd, ret);
        return ret;
    }

    printf("[Common][TCPSocket.cpp:%d][INFO]:Get_One_Code And Process_CallBack\n", __LINE__);
    if (!m_read_callback)
        return -1;
    ret = m_read_callback(*this);
    if (ret)
    {
        printf("[Common][TCPSocket.cpp:%d][ERROR]:m_read_callback failed. Error code = [%d]\n", __LINE__, ret);
        return ret;
    }
    return success;
}

//提交任务到io线程中
int TCPSocket::send(Functor temp)
{
    m_server->run_in_loop(temp);
    return 0;
}