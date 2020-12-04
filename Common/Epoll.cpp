//
// Created by kaiyu on 2020/10/27.
//

#include "Epoll.h"
#include <stdio.h>
#include <cstring>

Epoll::Epoll() {}

Epoll::~Epoll()
{
    close(m_epoll_fd);
    free(m_pevents);
}

int Epoll::epoll_init(int size)
{
    m_size = size;
    memset(&m_epoll_event, 0, sizeof(m_epoll_event));
    m_epoll_event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
    m_epoll_event.data.ptr = NULL;
    m_epoll_event.data.fd = -1;

    m_pevents = (struct epoll_event *)malloc(m_size * sizeof(struct epoll_event));
    if (NULL == m_pevents)
    {
        return -1;
    }

    m_epoll_fd = epoll_create(m_size);
    if (m_epoll_fd < 0)
    {
        return -1;
    }

    return 0;
}

int Epoll::epoll_wait(int timeout)
{
    return ::epoll_wait(m_epoll_fd, m_pevents, m_size, timeout);
}

int Epoll::epoll_add(int fd)
{
    if (fd < 0)
    {
        return -1;
    }
    m_epoll_event.data.fd = fd;
    printf("[Common][Epoll.cpp:%d]:Add Fd = [%d] !\n",__LINE__,fd);
    return epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &m_epoll_event);
}

epoll_event *Epoll::get_event(int idx)
{
    return &m_pevents[idx];
}