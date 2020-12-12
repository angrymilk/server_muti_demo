#include "../Common/SQLServer.h"
#include "../Common/base.h"
int main()
{
    std::shared_ptr<SQLServer> temp;
    temp.reset(new SQLServer);
    int id = 6989;
    for (int i = 1; i <= 100; i++)
    {
        temp->query(("UPDATE PlayerInfo SET port=3002 where user_id=" + std::to_string(id + i) + ";")
                        .c_str());
        /* temp->query(("INSERT INTO PlayerInfo (user_id,user_ip,port,user_name,user_password) VALUES (" +
                       std::to_string(id + i) + ",'127.0.0.1',3002,'chenzun" +
                       std::to_string(i) + "','123456')")
                          .c_str());
        */
    }
    return 0;
}