#include "GateServer.h"
#include <stdio.h>
#include <stdlib.h>
GateServer::GateServer()
{
    m_server.reset(new BaseServer("127.0.0.1", 8888));
    m_server->set_read_callback(std::bind(&GateServer::on_message, this, std::placeholders::_1));
    m_server->add_client_socket(8888, "127.0.0.1", 10022, "127.0.0.1");
    m_thread_task.Start();
}

int GateServer::run()
{
    if (m_server->run())
        return -101;
    return 0;
}

int GateServer::on_message(TCPSocket &con)
{
    //将函数扔入计算线程中
    m_thread_task.submit(std::bind(&GateServer::get_one_code, this, con));
    return 0;
}

void GateServer::get_one_code(TCPSocket &con)
{
    int ret = 0;
    while (1)
    {
        size_t data_size = MAX_SS_PACKAGE_SIZE;
        std::string m_sRvMsgBuf;
        m_sRvMsgBuf.reserve(MAX_SS_PACKAGE_SIZE);
        ret = con.m_buffer->get_one_code(const_cast<char *>(m_sRvMsgBuf.c_str()), data_size);
        if (ret > 0)
        {
            transmit(con, m_sRvMsgBuf, data_size);
            continue;
        }
        else if (ret < 0)
        {
            printf("[GateServer][GateServer.cpp:%d][ERROR]:get_one_code failed. errorCode:%d\n", __LINE__, ret);
        }
        break;
    }
}

void GateServer::transmit(TCPSocket &con, std::string &data, int datasize)
{
    int uid = 0;
    if (((datasize & BIT_COUNT) >> 20) == 3)
    {
        ClientDataQueryMessage req;
        req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, (datasize & ((1 << 20) - 1)) - 4);
        uid = req.uid();
    }
    else if (((datasize & BIT_COUNT) >> 20) == 4)
    {
        ClientDataChangeMessage req;
        req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, (datasize & ((1 << 20) - 1)) - 4);
        uid = req.uid();
    }
    else
    {
        printf("[GateServer][GateServer.cpp:%d][ERROR]:No Such Message Type\n", __LINE__);
        return;
    }

    if (m_player_con.find(uid) == m_player_con.end() && con.get_type == 1)
    {
        m_con_client[uid] = con.get_fd();
        m_player_con[uid] = m_con[uid % GateServer_NUM];
    }

    con.send(std::bind(&GateServer::send, this, con, data.c_str(), datasHize, uid));
}

void GateServer::send(TCPSocket &con, char *data, int size, int uid)
{
    if (con.get_type == 1)
    {
        m_server->m_sockets_map[m_player_con[uid]]->send_data(data, size);
    }
    else
    {
        if ((data & (1 << 30)))
        {
            for (unordered_map<int, int>::iterator iter = m_player_con.begin(); iter != m_player_con.end(); iter++)
            {
                m_server->m_sockets_map[m_con_client[uid]]->send_data(data, size);
                if (ret < success)
                {
                    printf("[GateServer][GateServer.cpp:%d][ERROR]:Send error ret=%d,errno:%d ,strerror:%s,fd = %d\n", __LINE__, ret, errno, strerror(errno), fd);
                }
                if (ret > success)
                {
                    printf("[GateServer][GateServer.cpp:%d][INFO]:Send try multi ret=%d, errno:%d ,strerror:%s, fd = %d\n", __LINE__, ret, errno, strerror(errno), fd);
                }
            }
        }
        else
            m_server->m_sockets_map[m_con_client[uid]]->send_data(data, size);
    }
}