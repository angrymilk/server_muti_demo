#include "LoginServer.h"
#include <stdio.h>
#include <stdlib.h>
LoginServer::LoginServer()
{
    m_sql_server.reset(new SQLServer);
    m_server.reset(new BaseServer("127.0.0.1", 10022));
    m_server->set_read_callback(std::bind(&LoginServer::on_message, this, std::placeholders::_1));
    m_con.resize(1);
    m_con[0] = m_server->add_client_socket(10022, "127.0.0.1", 3000, "127.0.0.1");
    m_gateserver_num = 1;
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
            if ((data_size & BIT_COUNT) == 0)
            {
                printf("[LoginServer][LoginServer.cpp:%d][INFO]: In Data Register Function\n", __LINE__);
                register_(con, m_sRvMsgBuf, data_size);
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

void LoginServer::register_(TCPSocket &con, std::string &data, int datasize)
{
    RegisterMessageOn req;
    req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, datasize);
    m_sql_server->query(("select user_id,user_password,port,user_ip from PlayerInfo where user_name='" + req.username() + "';").c_str());
    std::map<int, std::map<std::string, std::string>> password = m_sql_server->parser();
    std::cout << password[0]["user_password"] << "    " << req.password() << "\n";
    m_fd_map[std::stoi(password[0]["user_id"])] = con.get_fd();
    if (password[0]["user_password"] == req.password())
        printf("[LoginServer][LoginServer.cpp:%d][INFO]:密码匹配成功\n", __LINE__);
    else
        printf("[LoginServer][LoginServer.cpp:%d][ERROR]:密码匹配失败  ！！！！！！！！！！！！\n", __LINE__);
    m_con_map[std::stoi(password[0]["user_id"])] = 0; //目前只有一个，但是这里可以被扩展多个gateserver

    RegisterMessageBack res;
    res.set_ipaddr(password[0]["user_ip"]);
    res.set_port(std::stoi(password[0]["port"]));
    res.set_uid(std::stoi(password[0]["user_id"]));

    char data_[COMMON_BUFFER_SIZE];
    MsgHead head;
    head.m_message_len = (MESSAGE_HEAD_SIZE + res.ByteSize()) | (1 << 20);
    int temp = MESSAGE_HEAD_SIZE + res.ByteSize();
    int codeLength = 0;
    head.encode(data_, codeLength);
    res.SerializePartialToArray(data_ + MESSAGE_HEAD_SIZE, res.ByteSize());
    con.send(std::bind(&LoginServer::send_client, this, data_, temp, res.uid()));

    RegisterMessageGateBack res_gate;
    res_gate.set_password(password[0]["user_password"]);
    res_gate.set_uid(std::stoi(password[0]["user_id"]));
    MsgHead head_;
    head_.m_message_len = (MESSAGE_HEAD_SIZE + res_gate.ByteSize()) | (1 << 21);
    int temp_ = MESSAGE_HEAD_SIZE + res_gate.ByteSize();
    int codeLength_ = 0;
    head_.encode(data_ + temp, codeLength_);
    res_gate.SerializePartialToArray(data_ + temp + MESSAGE_HEAD_SIZE, res_gate.ByteSize());
    con.send(std::bind(&LoginServer::send_gate, this, data_ + temp, temp_, res_gate.uid()));
};

void LoginServer::send_client(char *data, int size, int uid)
{
    printf("--------------------------   %d  %d\n", m_fd_map[uid], size);
    m_server->m_sockets_map[m_fd_map[uid]]->send_data(data, size);
}

void LoginServer::send_gate(char *data, int size, int uid)
{
    printf("--------------------------   %d  %d\n", m_con[m_con_map[uid]], size);
    m_server->m_sockets_map[m_con[m_con_map[uid]]]->send_data(data, size);
}