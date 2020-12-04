#ifndef BUFFER_H
#define BUFFER_H
#include "base.h"

class Buffer
{
public:
    Buffer()
    {
        m_recv_buffer = new char[100000];
        m_recv_head = 0;
        m_recv_tail = 0;
        m_recv_buffer_size = 10240;
    }

    ~Buffer()
    {
        delete[] m_recv_buffer;
        m_recv_head = 0;
        m_recv_tail = 0;
    }
    int &get_tail();
    char &get_tail_point();
    int get_one_code(char *data, size_t &size);
    void reset();
    int copy_front();
    int get_recv_buffer_size();

private:
    int m_recv_head;
    int m_recv_tail;
    char *m_recv_buffer;
    int m_recv_buffer_size;
};
#endif