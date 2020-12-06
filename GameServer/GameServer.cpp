#include "GameServer.h"
#include <stdio.h>
#include <stdlib.h>
GameServer::GameServer()
{
    m_redis_server.reset(new RedisServer);
    m_sql_server.reset(new SQLServer);
    m_redis_server->Init();
    m_redis_server->Connect();

    m_server.reset(new BaseServer("127.0.0.1", 10022));
    m_server->set_read_callback(std::bind(&GameServer::on_message, this, std::placeholders::_1));
    m_thread_task.Start();
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
            if (((data_size & BIT_COUNT) >> 20) == 4)
            {
                printf("[GameServer][GameServer.cpp:%d][INFO]: In Data Change Function\n", __LINE__);
                solve_add(con, m_sRvMsgBuf, data_size);
            }
            else if (((data_size & BIT_COUNT) >> 20) == 3)
            {
                printf("[GameServer][GameServer.cpp:%d][INFO]: In Data Function\n", __LINE__);
                con.send_data
                    solve_query(con, m_sRvMsgBuf, data_size);
            }
            else if (((data_size & BIT_COUNT) >> 20) == 5)
            {
                printf("[GameServer][GameServer.cpp:%d][INFO]: In Move Function\n", __LINE__);
                move_calculate(con, m_sRvMsgBuf, data_size);
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

void GameServer::move_calculate(TCPSocket &con, std::string &data, int datasize)
{
    Player temp;
    parse(const_cast<char *>(data.c_str()), temp, datasize);
    handle_move(temp);
    char data_[COMMON_BUFFER_SIZE];
    serialize(data_, temp);
    vector<int> deletePlayer;
    for (unordered_map<int, PlayerInfo>::iterator iter = m_map_players.begin(); iter != m_map_players.end(); iter++)
    {
        int fd = m_map_players[iter->first].fd;
        if (m_server->m_sockets_map.find(fd) == m_server->m_sockets_map.end())
        {
            printf("[GameServer][GameServer.cpp:%d][WARNING]:fd:[%d] is not in the map now,maybe is deleted\n", __LINE__, fd);
            deletePlayer.emplace_back(iter->first);
            continue;
        }
    }
    for (vector<int>::iterator iter = deletePlayer.begin(); iter != deletePlayer.end(); iter++)
        m_map_players.erase(*iter);
    con.send(std::bind(&GameServer::send, this, data_, datasize));
}

void GameServer::serialize(char *data, Player &temp)
{
    //序列化处理
    ClientMoveMessage res;
    res.set_name(temp.uid);
    res.set_posx(temp.m_pos.posx);
    res.set_posy(temp.m_pos.posy);
    res.set_posz(temp.m_pos.posz);
    res.set_tarx(temp.m_pos.tarx);
    res.set_tary(temp.m_pos.tary);
    res.set_tarz(temp.m_pos.tarz);
    res.set_speed(temp.m_pos.speed);
    res.set_time(temp.m_pos.time);
    MsgHead head;
    head.m_message_len = res.ByteSize() + MESSAGE_HEAD_SIZE;
    int codeLength = 0;
    head.encode(data, codeLength);
    res.SerializePartialToArray(data + codeLength, res.ByteSize());
}

void GameServer::parse(char *input, Player &player, int &size)
{
    //反序列化处理
    ClientMoveMessage req;
    req.ParseFromArray(input + MESSAGE_HEAD_SIZE, ((size - MESSAGE_HEAD_SIZE) & ((1 << 20) - 1)));
    player.uid = req.uid();
    player.m_pos.posx = req.posx();
    player.m_pos.posy = req.posy();
    player.m_pos.posz = req.posz();
    player.m_pos.tarx = req.tarx();
    player.m_pos.tary = req.tary();
    player.m_pos.tarz = req.tarz();
    player.m_pos.speed = req.speed();
    player.m_pos.time = req.time();
}

void GameServer::handle_move(Player &player)
{
    int uid = player.get_id();
    Player player_ = m_player_vec[uid];
    int delta_time = player.time - player_.time;
    float move_rate = delta_time * player_.speed / sqrt((player.tarx - player_.tarx) * (player.tarx - player_.tarx) + (player.tarz - player_.tarz) * (player.tarz - player_.tarz));
    int x = player.tarx + move_rate * player.posx;
    int z = player.tarz + move_rate * player.posz;
    //int y = player.tary + move_rate * player.posy;
    if (10 < (x - player_.tarx) * (x - player_.tarx) + (z - player_.tarz) * (z - player_.tarz))
    {
        //如果差距太大做拉回的操作
        player.m_is_cheat = true;
        player.m_pos.posx = player_.posx;
        player.m_pos.posy = player.posy;
        player.m_pos.posz = player.posz;
        player.m_pos.tarx = player.tarx;
        player.m_pos.tary = player.tary;
        player.m_pos.tarz = player.tarz;
        player.m_pos.speed = player.speed;
    }
    else
        m_player_vec[uid] = player;
}

void GameServer::solve_add(TCPSocket &con, std::string &data, int datasize)
{
    int bodySize = (datasize & ((1 << 20) - 1)) - MESSAGE_HEAD_SIZE;
    ClientDataChangeMessage req;
    //Packageres res;
    req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, bodySize);
    if (m_player_db.find(req.uid()) == m_player_db.end())
    {
        m_player_db[req.uid()] = m_db[db_num % DB_NUM];
    }
    con.send(std::bind(&GameServer::send_db, this, data_, temp, uid));
}

void GameServer::solve_query(TCPSocket &con, std::string &data, int datasize)
{
    int bodySize = (datasize & ((1 << 20) - 1)) - MESSAGE_HEAD_SIZE;
    ClientDataQueryMessage req;
    //Packageres res;
    req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, bodySize);
    if (m_player_db.find(req.uid()) == m_player_db.end())
    {
        m_player_db[req.uid()] = m_db[db_num % DB_NUM];
    }
    con.send(std::bind(&GameServer::send_db, this, data_, temp, uid));
}

void GameServer::send(char *data, int size, int uid)
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

void GameServer::send_db(char *data, int size, int uid)
{
    if (con.get_type == 1)
    {
        m_server->m_sockets_map[db[uid]]->send_data(data, size);
    }
    else
    {
        if ((data & (1 << 30)))
        {
            for (unordered_map<int, int>::iterator iter = m_map_players.begin(); iter != m_map_players.end(); iter++)
            {
                m_server->m_sockets_map[db[uid]]->send_data(data, size);
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
            m_server->m_sockets_map[db[uid]]->send_data(data, size);
    }
}