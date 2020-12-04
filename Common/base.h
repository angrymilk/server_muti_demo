#ifndef DEMO_BASE_H
#define DEMO_BASE_H

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <memory>
#include <signal.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/select.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <pthread.h>
#include <sys/eventfd.h>

#endif

#include "../Proto/Demo.pb.h"
enum
{
    success = 0,
    fail = 1,
};

enum enmFdType
{
    fd_type_invalid = 0,
    fd_type_stdin = 1,
    fd_type_socket = 2,
    fd_type_listen = 3,
};

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888
typedef void (*ServerSendCallBack)(int uin, int cmd_id, google::protobuf::Message &msg);
//typedef void (*Functor)();
typedef std::function<void()> Functor;
enum
{
    MAX_SOCKET_COUNT = 10240,
};

enum
{
    MAX_SS_PACKAGE_SIZE = 0x10000,
    COMMON_BUFFER_SIZE = 0x10000, //2M大小，常规socket buffer缓存大小
};

enum enmConnErrorCode
{
    enmConnErrorCode_success = 0,
    enmConnErrorCode_invalid_socket = -1,
    enmConnErrorCode_recv_notenouth_buffer = -2,
    enmConnErrorCode_peer_closed = -3,
    enmConnErrorCode_send_notenouth_buffer = -4,
    enmConnErrorCode_invalid_param = -5,
    enmConnErrorCode_unknow = -6,

};

#endif //DEMO_BASE_H