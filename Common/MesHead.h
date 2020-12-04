#ifndef DEMO_MSGHEAD_H
#define DEMO_MSGHEAD_H

#include <cstdint>

#define MESSAGE_HEAD_SIZE (sizeof(int))
class MsgHead
{
public:
    MsgHead();
    ~MsgHead();
    MsgHead(MsgHead &head);

    int encode(char *pszOut, int &iOutLength);
    int decode(const char *pszIn, const int iInLength);

    int m_message_len;
};

#endif //DEMO_MSGHEAD_H
