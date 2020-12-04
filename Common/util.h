#ifndef DEMO_UTIL_H
#define DEMO_UTIL_H

#include <stdint.h>
#include <string>
#include "base.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int ip_string_to_addr(const char *string_ip, int &ip);
    int encode_int32(char **pOut, uint Src);
    int decode_int32(char **pIn, uint *pOut);
    int set_non_block(int fd);
    int connect_nonblock(int fd, struct sockaddr_in *serv_addr, socklen_t addrlen, int msecond);
    bool is_num(std::string type);

#ifdef __cplusplus
}
#endif

#endif //DEMO_UTIL_H
