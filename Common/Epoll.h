//
// Created by kaiyu on 2020/10/27.
//

#ifndef DEMO_EPOLL_H
#define DEMO_EPOLL_H

#include <stdint.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>

class Epoll
{
public:
    Epoll();
    ~Epoll();
    int epoll_init(int size);
    int epoll_wait(int timeout = -1);
    int epoll_add(int);
    struct epoll_event *get_event(int);

private:
    int m_size;
    int m_epoll_fd;
    struct epoll_event *m_pevents;
    struct epoll_event m_epoll_event;
};

#endif //DEMO_EPOLL_H
