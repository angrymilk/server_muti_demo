#include "GateServer.h"
#include <stdio.h>
#include <stdlib.h>
GateServer::GateServer()
{
    m_server.reset(new BaseServer("127.0.0.1", 3000));
    m_server->set_read_callback(std::bind(&GateServer::on_message, this, std::placeholders::_1));
    m_con.resize(1);
    m_con[0] = m_server->add_client_socket(3000, "127.0.0.1", 3001, "127.0.0.1");
}

int GateServer::run()
{
    if (m_server->run())
        return -101;
    return 0;
}

int GateServer::on_message(TCPSocket &con)
{
    get_one_code(con);
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

bool GateServer::check_user(int uid)
{
    if (m_client_check.find(uid) == m_client_check.end() || m_client_check[uid] == false)
        return false;
    return true;
}

void GateServer::transmit(TCPSocket &con, std::string &data, int datasize)
{
    int uid = 0;
    if (((datasize & BIT_COUNT) >> 20) == 3)
    {
        printf("Type == 3\n");
        ClientDataQueryMessage req;
        req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, (datasize & ((1 << 20) - 1)) - 4);
        uid = req.uid();
        if (m_user_pass[uid] != req.password())
        {
            std::cout << m_user_pass[uid] << "    " << req.password() << "\n";
            printf("[GateServer][GateServer.cpp:%d][ERROR]:Check Uers Error \n", __LINE__);
            return;
        }
    }
    else if (((datasize & BIT_COUNT) >> 20) == 4)
    {
        printf("Type == 4\n");
        ClientDataChangeMessage req;
        req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, (datasize & ((1 << 20) - 1)) - 4);
        uid = req.uid();
        if (m_user_pass[uid] != req.password())
        {
            printf("[GateServer][GateServer.cpp:%d][ERROR]:Check Uers Error \n", __LINE__);
            return;
        }
    }
    else if (((datasize & BIT_COUNT) >> 20) == 5)
    {
        printf("Type == 5\n");
        ClientMoveMessage req;
        req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, (datasize & ((1 << 20) - 1)) - 4);
        uid = req.uid();
        if (m_user_pass[uid] != req.password())
        {
            printf("[GateServer][GateServer.cpp:%d][ERROR]:Check Uers Error \n", __LINE__);
            return;
        }
    }
    else if (((datasize & BIT_COUNT) >> 20) == 2)
    {
        RegisterMessageGateBack req;
        req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, (datasize & ((1 << 20) - 1)) - 4);
        printf("Type == 2\n");
        //if (m_user_pass.find(req.uid()) == m_user_pass.end())
        m_user_pass[req.uid()] = req.password();
        //else
        //    m_client_check[req.uid()] = true;
        return;
    }
    else
    {
        printf("[GateServer][GateServer.cpp:%d][ERROR]:No Such Message Type\n", __LINE__);
        return;
    }

    if (m_player_con.find(uid) == m_player_con.end() && con.get_type() == 1)
    {
        m_con_client[uid] = con.get_fd();
        m_player_con[uid] = m_con[uid % GATESERVER_NUM];
    }

    con.send(std::bind(&GateServer::send, this, con, const_cast<char *>(data.c_str()), (datasize & ((1 << 20) - 1)), uid));
}

void GateServer::send(TCPSocket &con, char *data, int size, int uid)
{
    if (con.get_type() == 1)
    {
        m_server->m_sockets_map[m_player_con[uid]]->send_data(data, size);
    }
    else
    {
        if ((size & (1 << 30)))
        {
            for (unordered_map<int, int>::iterator iter = m_player_con.begin(); iter != m_player_con.end(); iter++)
            {
                m_server->m_sockets_map[m_con_client[uid]]->send_data(data, size);
            }
        }
        else
            m_server->m_sockets_map[m_con_client[uid]]->send_data(data, size);
    }
}