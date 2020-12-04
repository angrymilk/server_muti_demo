#include "LoginServer.h"
#include <stdio.h>
#include <stdlib.h>
LoginServer::LoginServer(std::string ip, int port)
{
    m_sql_server.reset(new SQLServer);
    m_send_info.ip = ip;
    m_send_info.port = port;

    m_server.reset(new BaseServer(ip, port));
    m_server->set_read_callback(std::bind(&LoginServer::on_message, this, std::placeholders::_1));
    //m_map_players[0].player = make_shared<Player>(0);
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
            if (((data_size & BIT_COUNT) >> 20) == 0)
            {
                printf("[LoginServer][LoginServer.cpp:%d][INFO]: In Data Register Function\n", __LINE__);
                register(con, m_sRvMsgBuf, data_size);
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
        m_sql_server->query(("UPDATE PlayerInfo SET user_id=" + std::to_string(m_name_map[req.name()]) + " WHERE user_name='" + req.name() + "';").c_str());
        m_map_players[m_name_map[req.name()]].fd = con.get_fd();
        m_map_players[m_name_map[req.name()]].player = make_shared<Player>(m_name_map[req.name()]);
    }

    RegisterMessageBack res;
    res.set_ipaddr(password[req.name()]["ip_address"]);
    res.set_port(password[req.name()]["port"]);
    res.set_uid(m_name_map[req.name()]);

    char data_[COMMON_BUFFER_SIZE];
    MsgHead head;
    head.m_message_len = res.ByteSize() + MESSAGE_HEAD_SIZE;
    int temp = head.m_message_len;
    int codeLength = 0;
    head.encode(data_, codeLength);
    res.SerializePartialToArray(data_ + MESSAGE_HEAD_SIZE, res.ByteSize());
    con.send(std::bind(&LoginServer::send, this, data_, temp, res.uid(), {password[req.name()]["port"], password[req.name()]["ip_address"]}));
}));
}

void LoginServer::serialize(TCPSocket &con, std::string &data, std::string &out, int type)
{
    //序列化处理
}

void LoginServer::parse(char *input, int &size, int type)
{
    //反序列化处理
}

void LoginServer::send(char *data, int size, int db_id, IpPort ip_port)
{
    db_id %= DB_NUM;
    std::shared_ptr<TCPSocket> db_socket = make_shared<TCPSocket>();
    db_socket->open_as_client(const_cast<char *>(ip_port.ip.c_str()), ip_port.port, m_clientinfo.bufferlen);

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