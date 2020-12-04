#ifndef __MYSQL_PLUS_H__
#define __MYSQL_PLUS_H__

#include <cstdint>
#include <string>

#include <mysql/mysql.h>
#include <map>
#include <string>

class SQLServer
{
private:
    MYSQL *con;

public:
    SQLServer();
    ~SQLServer();
    int32_t query(const char *queryer);

    std::map<std::string, std::map<std::string, std::string>> parser();
};

#endif