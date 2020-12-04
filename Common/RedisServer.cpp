#include "RedisServer.h"
#include <string.h>
RedisServer::RedisServer() : m_reply(nullptr), m_context(nullptr), m_pbuf(new char[512]), m_bufsize(512)
{
    std::cout << "RedisServer构造成功!" << std::endl;
}

RedisServer::~RedisServer()
{
    FreeReply();
    DisConnect();
    delete[] m_pbuf;
    std::cout << "RedisServer被析构了!" << std::endl;
}

int RedisServer::Init(const std::string &_ip, int _port)
{
    m_ip = _ip;
    m_port = _port;
    return 0;
}

int RedisServer::Restart()
{
    DisConnect();
    FreeReply();
    if (Connect())
    {
        return 0;
    }
    return -1;
}

int RedisServer::Connect()
{
    m_context = redisConnect(m_ip.c_str(), m_port);

    if (m_context == nullptr || m_context->err)
    {
        // error tracer log
        std::cout << "connect faild" << std::endl;
        return -1;
    }
    // demo 用的 redis 没有设置验证
    std::cout << "connect sucess" << std::endl;
    return 0;
}

int RedisServer::DisConnect()
{
    if (m_context)
    {
        redisFree(m_context);
        m_context = nullptr;
    }
    return 0;
}

int RedisServer::FreeReply()
{
    if (m_reply)
    {
        freeReplyObject(m_reply);
        m_reply = nullptr;
    }
    return 0;
}

int RedisServer::Set(const char *_key, int len, const char *_format)
{
    FreeReply();
    m_reply = (redisReply *)redisCommand(m_context, "SET %s %s", _key, _format);

    if (m_reply == nullptr || m_reply->type == REDIS_REPLY_ERROR)
    {
        std::cerr << "set faild " << std::endl;
        Restart();
        return -1;
    }
    return 0;
}

int RedisServer::Get(const char *_key, char *_buf, int *_len)
{
    FreeReply();

    m_reply = (redisReply *)redisCommand(m_context, "GET %s", _key);
    if (m_reply == nullptr || m_reply->type == REDIS_REPLY_ERROR)
    {
        std::cerr << "get faild" << std::endl;
        Restart();
        return -1;
    }

    if (m_reply->type == REDIS_REPLY_STRING)
    {
        strncpy(_buf, m_reply->str, m_reply->len);
        *_len = m_reply->len;
    }

    return 0;
}
