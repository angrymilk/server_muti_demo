#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>
#include "tinyxml2.h"

struct server_config
{
    std::string ip;
    uint32_t port;
};

struct sql_config
{
    std::string db;
    std::string ip;
    std::string usr;
    std::string pass;
};

int8_t load_config(const char *_name, server_config &_server);

int8_t load_config(const char *_name, sql_config &_sql);

#endif