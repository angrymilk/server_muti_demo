#include "GameServer.h"
#include <stdio.h>
#include <stdlib.h>
GameServer::GameServer(std::string ip, int port)
{
    m_redis_server.reset(new RedisServer);
    m_sql_server.reset(new SQLServer);
    m_redis_server->Init();
    m_redis_server->Connect();

    m_server.reset(new BaseServer(ip, port));
    m_server->set_read_callback(std::bind(&GameServer::on_message, this, std::placeholders::_1));
    m_thread_task.Start();
    m_map_players[0].fd = -1; //场景代表
    m_map_players[0].player = make_shared<Player>(0);
}

int GameServer::run()
{
    if (m_server->run())
        return -101;
    return 0;
}

int GameServer::on_message(TCPSocket &con)
{
    //将函数扔入计算线程中
    m_thread_task.submit(std::bind(&GameServer::get_one_code, this, con));
    return 0;
}

void GameServer::get_one_code(TCPSocket &con)
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
                printf("[GameServer][GameServer.cpp:%d][INFO]: In Data Change Function\n", __LINE__);
                solve_add(con, m_sRvMsgBuf, data_size);
            }
            else if (((data_size & ((1 << 20) | (1 << 21))) >> 20) == 2)
            {
                printf("[GameServer][GameServer.cpp:%d][INFO]: In Data Query Function\n", __LINE__);
                solve_query(con, m_sRvMsgBuf, data_size);
            }
            else if (((data_size & ((1 << 20) | (1 << 21))) >> 20) == 0)
            {
                printf("[GameServer][GameServer.cpp:%d][INFO]: In Data Register Function\n", __LINE__);
                regist(con, m_sRvMsgBuf, data_size);
            }
            continue;
        }
        else if (ret < 0)
        {
            printf("[GameServer][GameServer.cpp:%d][ERROR]:get_one_code failed. errorCode:%d\n", __LINE__, ret);
        }
        break;
    }
}

void GameServer::regist(TCPSocket &con, std::string &data, int datasize)
{
    Reqest req;
    req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, datasize);
    m_sql_server->query(("select user_name,user_password from PlayerInfo where user_name='" + req.name() + "';").c_str());
    std::map<std::string, std::map<std::string, std::string>> password = m_sql_server->parser();
    std::cout << password[req.name()]["user_password"] << "    " << req.password() << "\n";
    if (password[req.name()]["user_password"] == req.password())
        printf("[GameServer][GameServer.cpp:%d][INFO]:密码匹配成功\n", __LINE__);
    else
        printf("[GameServer][GameServer.cpp:%d][ERROR]:密码匹配失败  ！！！！！！！！！！！！\n", __LINE__);
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
    con.send(std::bind(&GameServer::send, this, data_, temp));
}

void GameServer::solve_add(TCPSocket &con, std::string &data, int datasize)
{
    //基本逻辑处理->调用con的发送函数
    int bodySize = (datasize & ((1 << 20) - 1)) - MESSAGE_HEAD_SIZE;
    Addreq req;
    req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, bodySize);
    if (req.value() > 0)
    {
        if (m_map_players.find(req.uid()) == m_map_players.end())
        {
            printf("[GameServer][GameServer.cpp:%d][ERROR]:No Such Player = [%d]\n", __LINE__, req.uid());
            return;
        }
        ItemInfo info;
        info.id = req.id();
        info.mmotype.resize(3);
        info.value.resize(3);
        info.mattrtype.resize(3);

        for (int i = 0; i < 3; i++)
        {
            info.mmotype[i] = (EltemModuleType)i;
            info.mattrtype[i].resize(3);
            info.value[i].resize(3);
            for (int j = 0; j < 3; j++)
            {
                info.value[i][j] = 0;
                info.mattrtype[i][j] = (EltemAttributeType)j;
            }
        }
        for (int i = 0; i < req.mode_size(); i++)
        {
            Modelinfo tmp = req.mode(i);
            for (int j = 0; j < tmp.attributetype_size(); j++)
                info.value[tmp.modeltype()][tmp.attributetype(j)] = tmp.attributetypevalue(j);
        }
        info.mtype = (EltemType)req.eltemtype();
        int ret = m_map_players[req.uid()].player->add(info, req.pos(), req.value(), req.dropfrom(), m_sql_server);
        if (ret)
            return;
    }
    else
    {
        m_map_players[req.uid()].player->consume(req.id(), (EltemType)req.eltemtype(), req.value(), req.dropfrom(), req.inuse(), m_sql_server);
    }
    //############################################  进行数据格式化方便进行redis的数据落地  ########################################################

    Redisplayerinfo tmp;
    tmp.set_hp(m_map_players[req.uid()].player->get_hp());
    tmp.set_attack(m_map_players[req.uid()].player->get_attack());
    tmp.set_id(req.uid());
    //存储目前正在使用的道具的信息
    if (m_map_players[req.uid()].player->m_in_use.begin() == m_map_players[req.uid()].player->m_in_use.end())
    {
        Attributeitempro *temp = tmp.add_inuse();
        temp->set_id(-1);
        temp->add_attribute(-1);
    }
    for (unordered_map<int, std::shared_ptr<AbstractItem>>::iterator iter = m_map_players[req.uid()].player->m_in_use.begin(); iter != m_map_players[req.uid()].player->m_in_use.end(); iter++)
    {
        printf("在使用中的道具\n");
        Attributeitempro *temp = tmp.add_inuse();
        std::shared_ptr<AbstractItem> ptr = m_map_players[req.uid()].player->m_in_use[iter->first];
        temp->set_amount(ptr->get_amount());
        temp->set_id(ptr->get_uid());
        temp->set_eltemtype(ptr->get_eltem_type());
        temp->add_attribute(ptr->get_attribute(EltemModuleType::eltem_Module_Base, EltemAttributeType::eltem_Attribute_Attack));
        temp->add_attribute(ptr->get_attribute(EltemModuleType::eltem_Module_Base, EltemAttributeType::eltem_Attribute_HP));
        temp->add_attribute(ptr->get_attribute(EltemModuleType::eltem_Module_Power, EltemAttributeType::eltem_Attribute_Attack));
        temp->add_attribute(ptr->get_attribute(EltemModuleType::eltem_Module_Power, EltemAttributeType::eltem_Attribute_HP));
        temp->add_attribute(ptr->get_attribute(EltemModuleType::eltem_Module_Insert, EltemAttributeType::eltem_Attribute_Attack));
        temp->add_attribute(ptr->get_attribute(EltemModuleType::eltem_Module_Insert, EltemAttributeType::eltem_Attribute_HP));
    }
    //存储背包相关的信息
    std::shared_ptr<Package> ptr = m_map_players[req.uid()].player->get_package();
    std::unordered_map<int, std::pair<int, int>> _map = ptr->get_map();
    if (_map.begin() != _map.end())
    {
        Packagepro *packagein = tmp.mutable_package();
        for (std::unordered_map<int, std::pair<int, int>>::iterator iter = _map.begin(); iter != _map.end(); iter++)
        {
            printf("存储背包相关的信息\n");
            Attributeitempro *temp = packagein->add_itempro();
            int i = iter->second.first, j = iter->second.second;
            temp->set_amount(ptr->get_vec(i, j)->get_amount());
            temp->set_id(ptr->get_vec(i, j)->get_uid());
            temp->set_eltemtype(ptr->get_vec(i, j)->get_eltem_type());
            temp->add_attribute(ptr->get_vec(i, j)->get_attribute(EltemModuleType::eltem_Module_Base, EltemAttributeType::eltem_Attribute_Attack));
            temp->add_attribute(ptr->get_vec(i, j)->get_attribute(EltemModuleType::eltem_Module_Base, EltemAttributeType::eltem_Attribute_HP));
            temp->add_attribute(ptr->get_vec(i, j)->get_attribute(EltemModuleType::eltem_Module_Power, EltemAttributeType::eltem_Attribute_Attack));
            temp->add_attribute(ptr->get_vec(i, j)->get_attribute(EltemModuleType::eltem_Module_Power, EltemAttributeType::eltem_Attribute_HP));
            temp->add_attribute(ptr->get_vec(i, j)->get_attribute(EltemModuleType::eltem_Module_Insert, EltemAttributeType::eltem_Attribute_Attack));
            temp->add_attribute(ptr->get_vec(i, j)->get_attribute(EltemModuleType::eltem_Module_Insert, EltemAttributeType::eltem_Attribute_HP));
        }
    }

    char key[5];
    char out[1001];
    snprintf(key, 5, "%d", tmp.id());
    tmp.SerializePartialToArray(out, tmp.ByteSize());
    m_redis_server->Set(key, tmp.ByteSize(), out);
    //############################################################  结束数据格式化  ##################################################################
    Response res;
    res.set_uid(req.uid());
    res.set_ack(1);

    char data_[COMMON_BUFFER_SIZE];
    MsgHead head;
    head.m_message_len = res.ByteSize() + MESSAGE_HEAD_SIZE;
    int temp = head.m_message_len;
    int codeLength = 0;
    head.encode(data_, codeLength);
    res.SerializePartialToArray(data_ + MESSAGE_HEAD_SIZE, res.ByteSize());
    con.send(std::bind(&GameServer::send, this, data_, temp));
}

void GameServer::solve_query(TCPSocket &con, std::string &data, int datasize)
{
    int bodySize = (datasize & ((1 << 20) - 1)) - MESSAGE_HEAD_SIZE;
    Packagereq req;
    //Packageres res;
    req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, bodySize);

    //查询redis端的数据
    if (req.init() == 1)
    {
        int id = req.uid();
        int len = 0;
        char idque[20];
        snprintf(idque, 5, "%d", id);
        char result[507];
        m_redis_server->Get(idque, result, &len);
        Redisplayerinfo tmp;
        tmp.ParseFromArray(result, len);
        printf("#################   从redis中读取客户端玩家的数据信息   ##################################\n");
        printf("#####################################################################################\n");
        printf("#####################################################################################\n");
        printf("################# Player_Id=%d      Player_Hp=%d    Player_Attakc=%d  ###############\n", tmp.id(), tmp.hp(), tmp.attack());
        for (int i = 0; i < tmp.inuse_size(); i++)
        {
            Attributeitempro temp = tmp.inuse(i);
            if (temp.id() == -1)
                break;
            printf("################# 正在处于使用中的道具   id=%d      道具数量=%d\n", temp.id(), temp.amount());
        }
        Packagepro packageinfo = tmp.package();
        for (int i = 0; i < packageinfo.itempro_size(); i++)
        {
            Attributeitempro temp = packageinfo.itempro(i);
            printf("################# 放在背包中的物品       id=%d      物品数量=%d   物品类型=%d (说明: 0:代表金钱  1:代表道具  2:代表消耗品)\n", temp.id(), temp.amount(), temp.eltemtype());
        }
        printf("#####################################################################################\n");
        printf("#####################################################################################\n");
    }

    Response res;
    res.set_ack(1);
    res.set_uid(req.uid());

    char data_[COMMON_BUFFER_SIZE];
    MsgHead head;
    head.m_message_len = res.ByteSize() + MESSAGE_HEAD_SIZE;
    int temp = head.m_message_len;
    int codeLength = 0;
    head.encode(data_, codeLength);
    res.SerializePartialToArray(data_ + MESSAGE_HEAD_SIZE, res.ByteSize());
    con.send(std::bind(&GameServer::send, this, data_, temp));
}

void GameServer::serialize(TCPSocket &con, std::string &data, std::string &out, int type)
{
    //序列化处理
}

void GameServer::parse(char *input, int &size, int type)
{
    //反序列化处理
}

void GameServer::send(char *data, int size)
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
            printf("[GameServer][GameServer.cpp:%d][ERROR]:Send error ret=%d,errno:%d ,strerror:%s,fd = %d\n", __LINE__, ret, errno, strerror(errno), fd);
        }
        if (ret > success)
        {
            printf("[GameServer][GameServer.cpp:%d][INFO]:Send try multi ret=%d, errno:%d ,strerror:%s, fd = %d\n", __LINE__, ret, errno, strerror(errno), fd);
        }
    }
}