#include "Buffer.h"

int Buffer::get_recv_buffer_size()
{
    return m_recv_buffer_size;
}

void Buffer::reset()
{
    if (m_recv_tail == m_recv_head)
    {
        m_recv_head = 0;
        m_recv_tail = 0;
    }
    return;
}

int Buffer::copy_front()
{
    int ret = 0;
    if (m_recv_tail == m_recv_buffer_size)
    {
        if (m_recv_head > 0)
        {
            //注意: 要使用memmove而不是memcpy
            //memcpy的__restrict__关键字不允许内存重叠
            memmove(&m_recv_buffer[0], &m_recv_buffer[m_recv_head], size_t(m_recv_tail - m_recv_head));
            m_recv_tail -= m_recv_head;
            m_recv_head = 0;
        }
        else
        {
            ret = (int)enmConnErrorCode_recv_notenouth_buffer;
            return ret;
        }
    }
    return ret;
}

int &Buffer::get_tail()
{
    return m_recv_tail;
}

char &Buffer::get_tail_point()
{
    return m_recv_buffer[m_recv_tail];
}

int Buffer::get_one_code(char *data, size_t &size)
{
    if (NULL == data)
    {
        return -1;
    }

    int buffer_data_size = m_recv_tail - m_recv_head;
    //判断接收缓冲区内的数据大小
    if (buffer_data_size <= 0)
    {
        return 0;
    }

    //根据表示长度的字节数，检查数据的合法性
    if (buffer_data_size < (int)sizeof(int))
    {
        if (m_recv_tail == m_recv_buffer_size)
        {
            memcpy(&m_recv_buffer[0], &m_recv_buffer[m_recv_head], size_t(buffer_data_size));
            m_recv_head = 0;
            m_recv_tail = buffer_data_size;
        }
        return 0;
    }

    //长度字段占用4byte
    int code_size = (int)ntohl((u_long)(*(int *)&m_recv_buffer[m_recv_head]));

    if (code_size < 0)
        return -2;

    if (!code_size)
    {
        std::cout << "No Data in Buffer............\n";
        return 0;
    }

    if ((code_size & ((1 << 20) - 1)) >= COMMON_BUFFER_SIZE)
    {
        size = (size_t)code_size;
        return -3;
    }

    //若接收缓冲区内的数据不足一个code
    if (buffer_data_size < (code_size & ((1 << 20) - 1)))
    {
        //并且数据已经存放到缓冲区尾了, 则移动数据到接收缓冲区头部
        //if (m_recv_tail == (int)sizeof(m_recv_buffer))
        if (m_recv_tail == m_recv_buffer_size)
        {
            memmove(&m_recv_buffer[0], &m_recv_buffer[m_recv_head], size_t(buffer_data_size));
            m_recv_head = 0;
            m_recv_tail = buffer_data_size;
        }
        return 0;
    }

    if ((int)size < (code_size & ((1 << 20) - 1)))
    {
        return -4;
    }

    size = (size_t)(code_size & ((1 << 20) - 1));
    //printf("+++++++++++++ -----------    %d\n", size);
    memcpy(data, &m_recv_buffer[m_recv_head], size);
    m_recv_head += size;
    size = (size_t)code_size;
    if (m_recv_tail == m_recv_head)
    {
        m_recv_head = 0;
        m_recv_tail = 0;
    }
    return 1;
}