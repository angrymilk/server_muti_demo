#include "GameServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../ThirdPart/loadconfig.h"
GameServer::GameServer()
{
    server_config game_conf;
    load_config("game_server", game_conf);
    m_server.reset(new BaseServer(game_conf.ip, game_conf.port));
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
    int ret = 0;
    while (1)
    {
        size_t data_size = MAX_SS_PACKAGE_SIZE;
        std::string m_sRvMsgBuf;
        m_sRvMsgBuf.reserve(MAX_SS_PACKAGE_SIZE);
        ret = con.m_buffer->get_one_code(const_cast<char *>(m_sRvMsgBuf.c_str()), data_size);
        if (ret > 0)
        {
            m_thread_task.submit(std::bind(&GameServer::run_function, this, con, m_sRvMsgBuf, data_size));
            continue;
        }
        else if (ret < 0)
        {
            printf("[GameServer][GameServer.cpp:%d][ERROR]:get_one_code failed. errorCode:%d\n", __LINE__, ret);
        }
        break;
    }
    return 0;
}

void GameServer::run_function(TCPSocket &con, std::string &out, int data_size)
{
    if (((data_size & BIT_COUNT) >> 20) == 4)
    {
        printf("[GameServer][GameServer.cpp:%d][INFO]: In Data Change Function\n", __LINE__);
        solve_add(con, out, data_size);
    }
    else if (((data_size & BIT_COUNT) >> 20) == 3)
    {
        printf("[GameServer][GameServer.cpp:%d][INFO]: In Data Query Function\n", __LINE__);
        solve_query(con, out, data_size);
    }
    else if (((data_size & BIT_COUNT) >> 20) == 5)
    {
        printf("[GameServer][GameServer.cpp:%d][INFO]: In Move Function\n", __LINE__);
        move_calculate(con, out, data_size);
    }
    else if (((data_size & BIT_COUNT) >> 20) == 7)
    {
        printf("[GameServer][GameServer.cpp:%d][INFO]: In DB Function\n", __LINE__);
        transmit_db(con, out, data_size);
    }
}

void GameServer::make_fd(int uid, int fd)
{
    if (m_map_players.find(uid) == m_map_players.end())
        m_map_players[uid] = {fd, make_shared<Player>(uid)};
}

void GameServer::move_calculate(TCPSocket &con, std::string &data, int datasize)
{
    Player temp(-1);
    parse(const_cast<char *>(data.c_str()), temp, datasize);
    make_fd(temp.uin(), con.get_fd());
    //handle_move(temp);
    SendHelp tt;
    /*
    srand((unsigned)time(NULL));
    for (int i = 0; i < 20; i++)
    {
        int temp = rand() % 10000000;
        temp /= 11357;
    }
    */
    serialize(tt.buf, temp, tt.datasize);
    /*
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
    */
    con.send(std::bind(&GameServer::send, this, tt, temp.uin()));
}

void GameServer::serialize(char *data, Player &temp, int &size)
{
    //序列化处理
    ClientMoveMessage res;
    res.set_uid(temp.uin());
    res.set_posx(temp.m_pos.posx);
    res.set_posy(temp.m_pos.posy);
    res.set_posz(temp.m_pos.posz);
    res.set_tarx(temp.m_pos.tarx);
    res.set_tary(temp.m_pos.tary);
    res.set_tarz(temp.m_pos.tarz);
    res.set_speed(temp.uin());
    res.set_time(temp.m_pos.time);
    res.set_password("123456");
    MsgHead head;
    head.m_message_len = ((res.ByteSize() + MESSAGE_HEAD_SIZE) | (1 << 20) | (1 << 22));
    size = res.ByteSize() + MESSAGE_HEAD_SIZE;
    int codeLength = 0;
    head.encode(data, codeLength);
    res.SerializePartialToArray(data + codeLength, res.ByteSize());
}

void GameServer::parse(char *input, Player &player, int &size)
{
    //反序列化处理
    ClientMoveMessage req;
    req.ParseFromArray(input + MESSAGE_HEAD_SIZE, ((size - MESSAGE_HEAD_SIZE) & ((1 << 20) - 1)));
    player.set_uin(req.uid());
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
    int uid = player.uin();
    std::shared_ptr<Player> player_ = m_map_players[uid].player;
    int delta_time = player.m_pos.time - player_->m_pos.time;
    float move_rate = delta_time * player_->m_pos.speed / sqrt((player.m_pos.posx - player_->m_pos.posx) * (player.m_pos.posx - player_->m_pos.posx) + (player.m_pos.posz - player_->m_pos.posz) * (player.m_pos.posz - player_->m_pos.posz));
    int x = player.m_pos.posx + move_rate * player.m_pos.tarx;
    int z = player.m_pos.posz + move_rate * player.m_pos.tarz;
    if (10 < (x - player_->m_pos.posx) * (x - player_->m_pos.posx) + (z - player_->m_pos.posz) * (z - player_->m_pos.posz))
    {
        //如果差距太大做拉回的操作
        player.m_pos.posx = player_->m_pos.posx;
        player.m_pos.posy = player_->m_pos.posy;
        player.m_pos.posz = player_->m_pos.posz;
        player.m_pos.tarx = player_->m_pos.tarx;
        player.m_pos.tary = player_->m_pos.tary;
        player.m_pos.tarz = player_->m_pos.tarz;
        player.m_pos.speed = player_->m_pos.speed;
    }
    else
        m_map_players[uid].player = std::make_shared<Player>(player);
}

void GameServer::solve_add(TCPSocket &con, std::string &data, int datasize)
{
    int bodySize = (datasize & ((1 << 20) - 1)) - MESSAGE_HEAD_SIZE;
    ClientDataChangeMessage req;
    req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, bodySize);
    if (m_player_db.find(req.uid()) == m_player_db.end())
    {
        m_player_db[req.uid()] = m_db[db_num % DB_NUM];
    }
    make_fd(req.uid(), con.get_fd());
    //con.send(std::bind(&GameServer::send_db, this, const_cast<char *>(data.c_str()), datasize, m_player_db[req.uid()]));
}

void GameServer::solve_query(TCPSocket &con, std::string &data, int datasize)
{
    int bodySize = (datasize & ((1 << 20) - 1)) - MESSAGE_HEAD_SIZE;
    ClientDataQueryMessage req;
    req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, bodySize);
    if (m_player_db.find(req.uid()) == m_player_db.end())
    {
        m_player_db[req.uid()] = m_db[db_num % DB_NUM];
    }
    make_fd(req.uid(), con.get_fd());
    //con.send(std::bind(&GameServer::send_db, this, const_cast<char *>(data.c_str()), datasize, m_player_db[req.uid()]));
}

void GameServer::transmit_db(TCPSocket &con, std::string &data, int datasize)
{
    int bodySize = (datasize & ((1 << 20) - 1)) - MESSAGE_HEAD_SIZE;
    Sqlplayerinfo req;
    req.ParseFromArray(const_cast<char *>(data.c_str()) + MESSAGE_HEAD_SIZE, bodySize);
    //con.send(std::bind(&GameServer::send_db, this, const_cast<char *>(data.c_str()), datasize, m_map_players[req.uid()].fd));
}

void GameServer::send(SendHelp data, int uin)
{
    m_server->m_sockets_map[m_map_players[uin].fd]->send_data(data.buf, data.datasize);
}

void GameServer::send_db(SendHelp data, int fd)
{
    m_server->m_sockets_map[fd]->send_data(data.buf, data.datasize);
}