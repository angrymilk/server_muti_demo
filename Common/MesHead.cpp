#include "base.h"
#include "MesHead.h"
#include "util.h"

MsgHead::MsgHead()
{
    m_message_len = 0;
}

MsgHead::~MsgHead()
{
}

MsgHead::MsgHead(MsgHead &head)
{
    m_message_len = head.m_message_len;
}

int MsgHead::encode(char *pszOut, int &iOutLength)
{
    if (NULL == pszOut)
    {
        return false;
    }
    char *ptmp = pszOut;
    iOutLength = encode_int32(&ptmp, (uint)m_message_len);
    return true;
}

int MsgHead::decode(const char *pszIn, const int iInLength)
{
    if (NULL == pszIn || iInLength <= 0)
    {
        return fail;
    }

    char *ptmp = const_cast<char *>(pszIn);
    decode_int32(&ptmp, (uint *)(&m_message_len));
    return success;
}