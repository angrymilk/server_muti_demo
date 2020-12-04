#include "BaseServer.h"

void BaseServer::do_pending_functions()
{
    printf("[Common][BaseServer.cpp:%d][INFO]:In do_pending_functions !!!\n", __LINE__);
    std::vector<Functor> functors;
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        functors.swap(m_pending_functor);
    }
    for (const Functor &functor : functors)
    {
        functor();
    }
    printf("[Common][BaseServer.cpp:%d][INFO]:Out do_pending_functions !!!\n", __LINE__);
}

int BaseServer::run()
{
    int ret = init();
    if (ret)
        return ret;

    for (;;)
    {
        if (epoll_recv())
            return -101;
    }
    return 0;
}

void BaseServer::run_in_loop(Functor func)
{
    //如果是在计算线程提交任务到了io线程中,那么就先放入io线程的pending队列中
    printf("[Common][BaseServer.cpp:%d][INFO]:I/O Thread Is Submitted Task From Other Thread !!!\n", __LINE__);
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_pending_functor.push_back(std::move(func));
    }
    wake_up_write();
}

void BaseServer::wake_up_write()
{
    uint64_t one = 1;
    ssize_t ret = ::write(m_wake_fd, &one, sizeof(one));
    if (ret < 0)
    {
        printf("[Common][BaseServer.cpp:%d][ERROR]:Write wake_up_fd error:[%d] errno:[%d]!!!\n", __LINE__, ret, errno);
        return;
    }
    printf("[Common][BaseServer.cpp:%d][INFO]:Write wake_up_fd num:[%d]\n", __LINE__, ret);
}

void BaseServer::wake_up_read()
{
    uint64_t one = 1;
    ssize_t ret = ::read(m_wake_fd, &one, sizeof(one));
    if (ret < 0)
    {
        printf("[Common][BaseServer.cpp:%d][ERROR]:Read wake_up_fd error errno:[%d]!!!\n", __LINE__, ret, errno);
        return;
    }
    printf("[Common][BaseServer.cpp:%d][INFO]:Read wake_up_fd num:[%d]\n", __LINE__, ret);
}

int BaseServer::init()
{
    m_thread_id = pthread_self();
    printf("[Common][BaseServer.cpp:%d][INFO]:Server Thread Num : [%d]\n", __LINE__, m_thread_id);
    if (m_server_socket->open_as_server(m_port, const_cast<char *>(m_ip.c_str())) < 0)
    {
        printf("[Common][BaseServer.cpp:%d][ERROR]:m_server_socket->open_as_server failed\n", __LINE__);
        return fail;
    }

    if (m_epoll.epoll_init(MAX_SOCKET_COUNT) < 0)
    {
        printf("[Common][BaseServer.cpp:%d][ERROR]:epoll_init failed\n", __LINE__);
        return fail;
    }
    printf("[Common][BaseServer.cpp:%d][INFO]:Server Epoll Has Inited...\n", __LINE__);

    if (m_epoll.epoll_add(m_server_socket->get_fd()) < 0)
    {
        printf("[Common][BaseServer.cpp:%d][ERROR]:m_epoll.epoll_add fd:%d failed\n", __LINE__, m_server_socket->get_fd());
        return fail;
    }

    if (m_epoll.epoll_add(m_wake_fd) < 0)
    {
        printf("[Common][BaseServer.cpp:%d][ERROR]:m_epoll.epoll_add wake_up_fd:%d failed\n", __LINE__, m_wake_fd);
        return fail;
    }

    m_sockets_map[m_server_socket->get_fd()] = m_server_socket;
    m_sockets_map[m_wake_fd] = make_shared<TCPSocket>(this);
    m_sockets_map[m_wake_fd]->set_socket_fd(m_wake_fd);
    return success;
}

void BaseServer::set_read_callback(TCPSocket::ReadFunctor read_func)
{
    m_read_func = read_func;
}

int BaseServer::epoll_recv()
{
    int fd_count = m_epoll.epoll_wait(-1);
    for (int i = 0; i < fd_count; i++)
    {
        struct epoll_event *pstEvent = m_epoll.get_event(i);
        int socketfd = pstEvent->data.fd;
        std::shared_ptr<TCPSocket> pstSocket = m_sockets_map[socketfd];
        if (pstSocket == NULL || pstSocket->get_fd() < 0)
        {
            printf("[Common][BaseServer.cpp:%d][ERROR]:get_server_tcpsocket failed fd:%d\n", __LINE__, socketfd);
            return fail;
        }

        if (pstSocket->get_fd() == m_server_socket->get_fd())
        {
            printf("[Common][BaseServer.cpp:%d][INFO]:Listening Fd Has New Fd\n", __LINE__);
            int accepted_sockfd = pstSocket->accept_fd(socketfd);
            if (accepted_sockfd < 0)
            {
                printf("[Common][BaseServer.cpp:%d][ERROR]:accept_fd  failed\n", __LINE__);
                continue;
            }

            m_sockets_map[accepted_sockfd] = make_shared<TCPSocket>(accepted_sockfd, this, m_read_func);

            if (m_epoll.epoll_add(accepted_sockfd) < 0)
            {
                printf("[Common][BaseServer.cpp:%d][ERROR]:m_epoll.epoll_add fd:%d failed\n", __LINE__, m_server_socket->get_fd());
                return fail;
            }
        }
        else if (pstSocket->get_fd() == m_wake_fd)
        {
            wake_up_read();
        }
        else
        {
            int ret = pstSocket->process_data();
            if (ret == -3)
            {
                printf("[Common][BaseServer.cpp:%d][WARNING]:socket fd:[%d] Has CLosed\n", __LINE__, socketfd);
                m_sockets_map.erase(socketfd);
            }
            else if (ret)
            {
                printf("[Common][BaseServer.cpp:%d][ERROR]:socket process_data failed fd:%d\n", __LINE__, pstSocket->get_fd());
                return ret;
            }
        }
    }
    //epoll处理完成之后进行对应的io task的任务读取
    do_pending_functions();
    return 0;
}