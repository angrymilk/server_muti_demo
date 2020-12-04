#include "util.h"
#include <poll.h>
#include <cstring>
int ip_string_to_addr(const char *string_ip, int &ip)
{
    struct in_addr stAddr;
    if (!string_ip || inet_pton(AF_INET, string_ip, &stAddr) <= 0)
    {
        return -1;
    }
    ip = (int)stAddr.s_addr;
    return 0;
}

int encode_int32(char **pOut, uint Src)
{
    if (NULL == pOut || NULL == *pOut)
    {
        return 0;
    }

    *(uint *)(*pOut) = htonl(Src);
    *pOut += sizeof(uint);
    return int(sizeof(uint));
}

int decode_int32(char **pIn, uint *pOut)
{
    if (NULL == pIn || NULL == *pIn || NULL == pOut)
    {
        return 0;
    }
    *pOut = (uint)ntohl((uint) * (uint *)(*pIn));
    *pIn += sizeof(uint);
    return int(sizeof(uint));
}

int set_non_block(int fd)
{
    int flag = 0;
    flag = fcntl(fd, F_GETFL, 0);
    if (flag < 0)
    {
        return -1;
    }

    flag |= O_NONBLOCK;
    flag |= O_NDELAY;

    if (fcntl(fd, F_SETFL, flag) < 0)
    {
        return -1;
    }

    return 0;
}

int connect_nonblock(int fd, struct sockaddr_in *serv_addr, socklen_t addrlen, int msecond)
{
    if (msecond < 0)
    {
        return -1;
    }

    int flag = 0;
    flag = fcntl(fd, F_GETFL, 0);
    if (flag < 0)
    {
        return -1;
    }

    if (fcntl(fd, F_SETFL, (flag | O_NONBLOCK)) < 0)
    {
        return -1;
    }

    int error_code = 0;
    int ret = 0;

    ret = connect(fd, (const sockaddr *)serv_addr, addrlen);
    if (ret < 0)
    {
        if (errno != EINPROGRESS)
        {
            return -1;
        }

        //on connecting
        struct pollfd stPollFD;
        memset(&stPollFD, 0, sizeof(stPollFD));
        stPollFD.fd = fd;
        stPollFD.events = POLLIN | POLLOUT;

        ret = poll(&stPollFD, 1, msecond);
        if (0 >= ret)
        {
            close(fd);
            return -1;
        }

        //when some socket fd was read(ret > 0)
        if (stPollFD.revents & POLLIN || stPollFD.revents & POLLOUT)
        {
            socklen_t len = (socklen_t)sizeof(error_code);
            if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error_code, &len) < 0)
            {
                close(fd);
                return -1;
            }
        }
        else
        {
            close(fd);
            return -1;
        }
    }

    fcntl(fd, F_SETFL, flag);
    if (error_code)
    {
        close(fd);
        return -1;
    }

    return 0;
}

bool is_num(std::string type)
{
    if (type.size() > 10 || type.size() <= 1)
        return false;
    for (size_t i = 0; i < type.size() - 1; i++)
    {
        if (type[i] < '0' || type[i] > '9')
            return false;
    }
    return true;
}