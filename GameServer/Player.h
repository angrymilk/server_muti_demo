#ifndef PLAYER_H
#define PLAYER_H
#include "../Common/base.h"
#include "../Common/MesHead.h"
#include "../Common/Epoll.h"
#include "../Common/MesHandle.h"
#include <map>
#include <cstdio>
#include "../Package/Package.h"
#include "../Common/SQLServer.h"

class Player
{
public:
    Player(int uin)
    {
        m_uin = uin;
        m_package.reset(new Package);
        m_attack = 100;
        m_hp = 100;
    }

    ~Player();

    void set_uin(int id);

    int uin();

    int add(ItemInfo info, int pos, int value, bool to_package, std::shared_ptr<SQLServer> sql_server);

    int consume(int id, EltemType type, int value, bool to_package, bool inuse, std::shared_ptr<SQLServer> sql_server);

    int get_num(int id);

    int get_hp();

    int get_attack();

    std::shared_ptr<Package> get_package();
    std::unordered_map<int, std::shared_ptr<AbstractItem>> m_in_use; //在使用中的道具(非背包部分的道具)

private:
    int m_uin;
    std::shared_ptr<Package> m_package; //非使用中的道具(背包中的道具)
    int m_attack;
    int m_hp;
};
#endif