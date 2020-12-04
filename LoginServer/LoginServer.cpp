#include "LoginServer.h"
#include <stdio.h>
#include <stdlib.h>
LoginServer::LoginServer(std::string ip, int port)
{
    m_redis_server.reset(new RedisServer);
    m_sql_server.reset(new SQLServer);
    m_redis_server->Init();
    m_redis_server->Connect();

    m_server.reset(new BaseServer(ip, port));
    m_server->set_read_callback(std::bind(&LoginServer::on_message, this, std::placeholders::_1));
    m_map_players[0].fd = -1; //场景代表
    m_map_players[0].player = make_shared<Player>(0);
}

int LoginServer::run()
{
    if (m_server->run())
        return -101;
    return 0;
}

int LoginServer::on_message(TCPSocket &con)
{
    get_one_code(con);
    return 0;
}

void LoginServer::get_one_code(TCPSocket &con)
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
            if (((data_size & ((1 << 20) | (1 << 21))) >> 20) == 1)
            {
                printf("[LoginServer][LoginServer.cpp:%d][INFO]: In Data Change Function\n", __LINE__);
                solve_add(con, m_sRvMsgBuf, data_size);
            }
            else if (((data_size & ((1 << 20) | (1 << 21))) >> 20) == 2)
            {
                printf("[LoginServer][LoginServer.cpp:%d][INFO]: In Data Query Function\n", __LINE__);
                solve_query(con, m_sRvMsgBuf, data_size);
            }
            else if (((data_size & ((1 << 20) | (1 << 21))) >> 20) == 0)
            {
                printf("[LoginServer][LoginServer.cpp:%d][INFO]: In Data Register Function\n", __LINE__);
                regist(con, m_sRvMsgBuf, data_size);
            }
            continue;
        }
        else if (ret < 0)
        {
            printf("[LoginServer][LoginServer.cpp:%d][ERROR]:get_one_code failed. errorCode:%d\n", __LINE__, ret);
        }
        break;
    }
}

void LoginServer::register(TCPSocket &con, std::string &data, int datasize)
{
    Reqest req;
    req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, datasize);
    m_sql_server->query(("select user_name,user_password from PlayerInfo where user_name='" + req.name() + "';").c_str());
    std::map<std::string, std::map<std::string, std::string>> password = m_sql_server->parser();
    std::cout << password[req.name()]["user_password"] << "    " << req.password() << "\n";
    if (password[req.name()]["user_password"] == req.password())
        printf("[LoginServer][LoginServer.cpp:%d][INFO]:密码匹配成功\n", __LINE__);
    else
        printf("[LoginServer][LoginServer.cpp:%d][ERROR]:密码匹配失败  ！！！！！！！！！！！！\n", __LINE__);
    if (m_name_map.find(req.name()) == m_name_map.end())
    {
        m_name_map[req.name()] = rand() % 10009;
        m_map_players[m_name_map[req.name()]].fd = con.get_fd();
        m_map_players[m_name_map[req.name()]].player = make_shared<Player>(m_name_map[req.name()]);
    }
    m_sql_server->query(("UPDATE PlayerInfo SET user_id=" + std::to_string(m_name_map[req.name()]) + " WHERE user_name='" + req.name() + "';").c_str());
    m_sql_server->query(("UPDATE PlayerInfo SET hp=" + std::to_string(100) + " WHERE user_name='" + req.name() + "';").c_str());
    m_sql_server->query(("UPDATE PlayerInfo SET attack=" + std::to_string(100) + " WHERE user_name='" + req.name() + "';").c_str());

    Response res;
    res.set_uid(m_name_map[req.name()]);
    res.set_ack(1);

    char data_[COMMON_BUFFER_SIZE];
    MsgHead head;
    head.m_message_len = res.ByteSize() + MESSAGE_HEAD_SIZE;
    int temp = head.m_message_len;
    int codeLength = 0;
    head.encode(data_, codeLength);
    res.SerializePartialToArray(data_ + MESSAGE_HEAD_SIZE, res.ByteSize());
    con.send(std::bind(&LoginServer::send, this, data_, temp));
}

void LoginServer::serialize(TCPSocket &con, std::string &data, std::string &out, int type)
{
    //序列化处理
}

void LoginServer::parse(char *input, int &size, int type)
{
    //反序列化处理
}

void LoginServer::send(char *data, int size)
{
    int ret = 0;
    for (unordered_map<int, PlayerInfo>::iterator iter = m_map_players.begin(); iter != m_map_players.end(); iter++)
    {
        int fd = m_map_players[iter->first].fd;
        if (fd == -1)
            continue;
        ret = m_server->m_sockets_map[fd]->send_data(data, size);
        if (ret < success)
        {
            printf("[LoginServer][LoginServer.cpp:%d][ERROR]:Send error ret=%d,errno:%d ,strerror:%s,fd = %d\n", __LINE__, ret, errno, strerror(errno), fd);
        }
        if (ret > success)
        {
            printf("[LoginServer][LoginServer.cpp:%d][INFO]:Send try multi ret=%d, errno:%d ,strerror:%s, fd = %d\n", __LINE__, ret, errno, strerror(errno), fd);
        }
    }
}