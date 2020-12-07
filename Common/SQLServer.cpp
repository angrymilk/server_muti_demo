#include "SQLServer.h"
#include <stdio.h>
#include <stdlib.h>

SQLServer::SQLServer()
{
    con = mysql_init(NULL);
    if (con == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(con));
    }

    if (!mysql_real_connect(con, "localhost", "root", "12345678", "PlayerInfo", 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
    }
    printf("SQLServer Is Started !!!\n");
}

SQLServer::~SQLServer() {}

int32_t SQLServer::query(const char *queryer)
{
    if (mysql_query(con, queryer))
    {
        fprintf(stderr, "%s\n", mysql_error(con));
        return -1;
    }
    return 0;
}

std::map<int, std::map<std::string, std::string>> SQLServer::parser()
{
    std::map<int, std::map<std::string, std::string>> mp;
    MYSQL_RES *res = mysql_store_result(con);
    int32_t num_fields = mysql_num_fields(res);
    MYSQL_FIELD *field = mysql_fetch_fields(res);
    int index;
    int num = 0;
    while (MYSQL_ROW row = mysql_fetch_row(res))
    {
        for (int32_t i = 0; i < num_fields; ++i)
        {
            if (i == 0)
            {
                //mp.insert(0, std::map<std::string, std::string>());
                index = num;
            }
            else
            {
                mp[index].insert(std::make_pair(field[i].name, row[i]));
            }
            printf("[%s]=>[%s]\n", field[i].name, row[i]);
        }
        num++;
    }
    mysql_free_result(res);
    return mp;
}