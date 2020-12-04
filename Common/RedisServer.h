#ifndef REDISSERVER_H_
#define REDISSERVER_H_

#include <vector>
#include <string>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <hiredis/hiredis.h>
class RedisServer
{
public:
    RedisServer();
    ~RedisServer();

    int Init(const std::string &_ip = "127.0.0.1", int port = 6379);

    int Connect();

    int DisConnect();

    int FreeReply();

    int Restart();

    template <typename T>
    int SetByString(const std::string &key, T &_t)
    {
        FreeReply();

        m_ss << "SET " << key << " " << _t;
        m_ss >> m_cmd;

        m_reply = (redisReply)redisCommand(m_context, m_cmd.c_str());

        if (m_reply == nullptr || m_reply->type != REDIS_REPLY_STATUS)
        {
            // error
            Restart();
        }
        return 0;
    }

    int Set(const char *_key, int len, const char *format);

    int Get(const char *_key, char *_buf, int *_len);

private:
    std::stringstream m_ss;
    std::string m_cmd;

    redisReply *m_reply;
    redisContext *m_context;

    std::string m_ip;
    int m_port;

    char *m_pbuf;
    int m_bufsize;
};

#endif