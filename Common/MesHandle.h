#ifndef DEMO_MSGHANDLER_H
#define DEMO_MSGHANDLER_H

#include "stdint.h"
#include "MesHead.h"
#include <map>

template <class T>
class MsgHandler
{
public:
    typedef int (T::*funHandle)(std::function<void(int uin, int cmd_id, google::protobuf::Message &msg)> ServerSend, MsgHead &stHead, const char *body, const int len);

private:
    std::map<int, funHandle> m_mapRequest;

public:
    void RegisterMsg(int MessageID, funHandle func)
    {
        if (m_mapRequest.find(MessageID) != m_mapRequest.end())
        {
            printf("[MsgHandler::%s] conflict client msg id(id:%d)\n", __FUNCTION__, MessageID);
            exit(0);
        }
        m_mapRequest[MessageID] = func;
    }
    funHandle get_func(int MessageID)
    {
        auto it = m_mapRequest.find(MessageID);
        if (it != m_mapRequest.end())
        {
            return it->second;
        }
        printf("[MsgHandler::%s] iMessageID=(%d | 0x%x) is not register\n", __FUNCTION__, MessageID, MessageID);
        return NULL;
    }
};

#endif //DEMO_MSGHANDLER_H