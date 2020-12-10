#include "DBServer.h"
#include <stdio.h>
#include <stdlib.h>
#include "../ThirdPart/loadconfig.h"
DBServer::DBServer()
{
    server_config db_conf;
    load_config("db_server", db_conf);

    m_sql_server.reset(new SQLServer);
    m_server.reset(new BaseServer(db_conf.ip, db_conf.port));
    m_server->set_read_callback(std::bind(&DBServer::on_message, this, std::placeholders::_1));
    m_con.resize(1);

    server_config game_conf;
    load_config("game_server", game_conf);
    m_con[0] = m_server->add_client_socket(db_conf.port, db_conf.ip, game_conf.port, game_conf.ip);
}

int DBServer::run()
{
    if (m_server->run())
        return -101;
    return 0;
}

int DBServer::on_message(TCPSocket &con)
{
    get_one_code(con);
    return 0;
}

void DBServer::get_one_code(TCPSocket &con)
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
            if (((data_size & BIT_COUNT) >> 20) == 4)
            {
                printf("[DBServer][DBServer.cpp:%d][INFO]: In Data Change Function\n", __LINE__);
                solve_add(con, m_sRvMsgBuf, data_size);
            }
            else if (((data_size & BIT_COUNT) >> 20) == 3)
            {
                printf("[DBServer][DBServer.cpp:%d][INFO]: In Data Function\n", __LINE__);
                solve_query(con, m_sRvMsgBuf, data_size);
            }
            continue;
        }
        else if (ret < 0)
        {
            printf("[DBServer][DBServer.cpp:%d][ERROR]:get_one_code failed. errorCode:%d\n", __LINE__, ret);
        }
        break;
    }
}

//物品的增和删
void DBServer::solve_add(TCPSocket &con, std::string &data, int datasize)
{
    int bodySize = (datasize & ((1 << 20) - 1)) - MESSAGE_HEAD_SIZE;
    ClientDataChangeMessage req;
    req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, bodySize);
    m_player_con[req.uid()] = con.get_fd();
    if (req.value() > 0)
    {
        if (req.inuse())
            m_sql_server->query(("INSERT INTO UseInfo (player_id,item_id,item_num) VALUES ('" + std::to_string(req.uid()) + "','" + std::to_string(req.id()) + "','" + std::to_string(1) + "')").c_str());
        else
        {
            m_sql_server->query(("select * from PackageInfo where user_id='" + std::to_string(req.uid()) + "' AND item_id= '" + std::to_string(req.id()) + "';").c_str());
            std::map<int, std::map<std::string, std::string>> iteminfo = m_sql_server->parser();
            if (iteminfo.size())
            {
                int num = std::stoi(iteminfo[0]["item_num"]);
                num += req.value();
                m_sql_server->query(("UPDATE PackageInfo SET item_num='" + std::to_string(num) + "' WHERE user_id='" + std::to_string(req.uid()) + "' AND item_id= '" + std::to_string(req.id()) + "';").c_str());
            }
            else
                m_sql_server->query(("INSERT INTO PackageInfo (player_id,item_id,item_num) VALUES ('" + std::to_string(req.uid()) + "','" + std::to_string(req.id()) + "','" + std::to_string(1) + "')").c_str());
        }
    }
    else
    {
        if (req.dropfrom())
        {
            m_sql_server->query(("DELETE FROM UseInfo WHRER item_id='" + std::to_string(req.id()) + "' AND user_id='" + std::to_string(req.uid()) + "')").c_str());
        }
        else
        {
            m_sql_server->query(("select * from PackageInfo where user_id='" + std::to_string(req.uid()) + "' AND item_id= '" + std::to_string(req.id()) + "';").c_str());
            std::map<int, std::map<std::string, std::string>> iteminfo = m_sql_server->parser();
            int num = std::stoi(iteminfo[0]["item_num"]);
            num += req.value();
            if (num)
            {
                m_sql_server->query(("UPDATE PackageInfo SET item_num='" + std::to_string(num) + "' WHERE user_id='" + std::to_string(req.uid()) + "' AND item_id= '" + std::to_string(req.id()) + "';").c_str());
            }
            else
                m_sql_server->query(("DELETE FROM PackageInfo WHERE player_id='" + std::to_string(req.uid()) + "' AND item_id='" + std::to_string(req.id()) + "')").c_str());
        }
    }
}

//物品的信息查询
void DBServer::solve_query(TCPSocket &con, std::string &data, int datasize)
{
    ClientDataQueryMessage req;
    Sqlplayerinfo res;
    int bodySize = (datasize & ((1 << 20) - 1)) - MESSAGE_HEAD_SIZE;
    req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, bodySize);

    //玩家个人信息的数据加载
    m_sql_server->query(("select * from PlayerInfo where user_id='" + std::to_string(req.uid()) + "';").c_str());
    std::map<int, std::map<std::string, std::string>> userinfo = m_sql_server->parser();
    res.set_hp(stoi(userinfo[0]["hp"]));
    res.set_attack(stoi(userinfo[0]["attack"]));
    res.set_uid(req.uid());

    //道具背包的数据加载
    m_sql_server->query(("select * from UseInfo where user_id='" + std::to_string(req.uid()) + "';").c_str());
    std::map<int, std::map<std::string, std::string>> useinfo = m_sql_server->parser();
    for (int i = 0;; i++)
    {
        if (useinfo.find(i) == useinfo.end())
            break;
        Attributeitempro *temp = res.add_inuse();
        int item_id = std::stoi(useinfo[i]["item_id"]);
        temp->set_id(item_id);
        temp->set_amount(std::stoi(useinfo[i]["item_num"]));
        m_sql_server->query(("select * from ItemInfo where item_id='" + std::to_string(item_id) + "';").c_str());
        std::map<int, std::map<std::string, std::string>> iteminfo = m_sql_server->parser();
        temp->set_eltemtype(std::stoi(iteminfo[0]["item_type"]));
        temp->add_attribute(std::stoi(iteminfo[0]["eltem_Module_Base_Hp"]));
        temp->add_attribute(std::stoi(iteminfo[0]["eltem_Module_Base_Attack"]));
        temp->add_attribute(std::stoi(iteminfo[0]["eltem_Module_Power_Hp"]));
        temp->add_attribute(std::stoi(iteminfo[0]["eltem_Module_Power_Attack"]));
        temp->add_attribute(std::stoi(iteminfo[0]["eltem_Module_Insert_Hp"]));
        temp->add_attribute(std::stoi(iteminfo[0]["eltem_Module_Insert_Attack"]));
    }

    //背包的数据加载
    Packagepro *pack = res.mutable_package();
    m_sql_server->query(("select * from PackageInfo where user_id='" + std::to_string(req.uid()) + "';").c_str());
    std::map<int, std::map<std::string, std::string>> packageinfo = m_sql_server->parser();
    for (int i = 0;; i++)
    {
        if (packageinfo.find(i) == packageinfo.end())
            break;
        Attributeitempro *temp = pack->add_itempro();
        int item_id = std::stoi(packageinfo[i]["item_id"]);
        temp->set_id(item_id);
        temp->set_amount(std::stoi(packageinfo[i]["item_num"]));
        m_sql_server->query(("select * from ItemInfo where item_id='" + std::to_string(item_id) + "';").c_str());
        std::map<int, std::map<std::string, std::string>> iteminfo = m_sql_server->parser();
        temp->set_eltemtype(std::stoi(iteminfo[0]["item_type"]));
        temp->add_attribute(std::stoi(iteminfo[0]["eltem_Module_Base_Hp"]));
        temp->add_attribute(std::stoi(iteminfo[0]["eltem_Module_Base_Attack"]));
        temp->add_attribute(std::stoi(iteminfo[0]["eltem_Module_Power_Hp"]));
        temp->add_attribute(std::stoi(iteminfo[0]["eltem_Module_Power_Attack"]));
        temp->add_attribute(std::stoi(iteminfo[0]["eltem_Module_Insert_Hp"]));
        temp->add_attribute(std::stoi(iteminfo[0]["eltem_Module_Insert_Attack"]));
    }

    char data_[COMMON_BUFFER_SIZE];
    MsgHead head;
    int temp = head.m_message_len;
    head.m_message_len = ((res.ByteSize() + MESSAGE_HEAD_SIZE) ^ (1 << 23));
    int codeLength = 0;
    head.encode(data_, codeLength);
    res.SerializePartialToArray(data_ + MESSAGE_HEAD_SIZE, res.ByteSize());
    con.send(std::bind(&DBServer::send, this, data_, temp, req.uid()));
}

void DBServer::send(char *data, int size, int uid)
{
    m_server->m_sockets_map[m_player_con[uid]]->send_data(data, size);
}