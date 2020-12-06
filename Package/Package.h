#ifndef PACKAGE_H
#define PACKAGE_H
#include "Money.h"
#include "Consume.h"
#include "Equip.h"
#include "ItemFactory.h"
#include <vector>
#include "../Common/SQLServer.h"
class Package
{
public:
    Package()
    {
        m_vec.resize(3);
        m_num.resize(3);
        m_num[0] = m_num[1] = m_num[2] = 0;
        m_vec[0].resize(1);
        m_vec[1].resize(10);
        m_vec[2].resize(10);
        //m_vec[0][0] = m_factory.create(info, 0);
    }
    //这里的value代表的是物品个数，info中的value代表的是物品和道具属性的值
    //如果物品之前已经在背包中有同样id的，那么直接就重叠，不然就在客户端传进来的pos中放入新的类型的背包
    int add(ItemInfo info, int pos, int value, std::shared_ptr<SQLServer> sql_server)
    {
        int id = 0;
        if (info.mtype == EltemType::eMoney)
        {
            if (m_vec[0][0] == nullptr)
            {
                m_vec[0][0] = m_factory.create(info, 0);
                m_posmap[info.id] = std::make_pair(0, 0);
                m_vec[0][0]->set_amount(value);
                sql_server->query(("INSERT INTO PackageInfo ( player_id, item_id,item_num ) VALUES ( " + std::to_string(uid) + "," + std::to_string(info.id) + "," + std::to_string(0) + ");").c_str());
            }
            else
            {
                int tt = m_vec[0][0]->get_amount();
                m_vec[0][0]->set_amount(tt + value);
                sql_server->query(("UPDATE PackageInfo SET item_num=" + std::to_string(value) + " WHERE item_id=" + std::to_string(info.id) + ";").c_str());
            }
        }
        else if (info.mtype == EltemType::eEQUIP)
        {
            if (m_posmap.find(info.id) != m_posmap.end())
            {
                if (m_num[1] == 10)
                    return -1;
                pos = m_posmap[info.id].second;
                m_vec[1][pos]->set_amount(value + m_vec[1][pos]->get_amount());
                sql_server->query(("UPDATE PackageInfo SET item_num=" + std::to_string(m_vec[1][pos]->get_amount()) + " WHERE item_id=" + std::to_string(info.id) + ";").c_str());
            }
            else
            {
                m_vec[1][pos] = m_factory.create(info, 0);
                m_posmap[info.id] = std::make_pair(1, pos);
                m_vec[1][pos]->set_amount(value);
                sql_server->query(("INSERT INTO PackageInfo ( player_id, item_id,item_num ) VALUES ( " + std::to_string(uid) + "," + std::to_string(info.id) + "," + std::to_string(value) + ");").c_str());
            }
        }
        else if (info.mtype == EltemType::eCONSUME)
        {
            if (m_posmap.find(info.id) != m_posmap.end())
            {
                if (m_num[2] == 10)
                    return -1;
                pos = m_posmap[info.id].second;
                m_vec[2][pos]->set_amount(value + m_vec[2][pos]->get_amount());
                sql_server->query(("UPDATE PackageInfo SET item_num=" + std::to_string(m_vec[2][pos]->get_amount()) + " WHERE item_id=" + std::to_string(info.id) + ";").c_str());
            }
            else
            {
                m_vec[2][pos] = m_factory.create(info, 0);
                m_posmap[info.id] = std::make_pair(2, pos);
                m_vec[2][pos]->set_amount(value);
                sql_server->query(("INSERT INTO PackageInfo ( player_id, item_id,item_num ) VALUES ( " + std::to_string(uid) + "," + std::to_string(info.id) + "," + std::to_string(value) + ");").c_str());
            }
        }
        else
            return -1;
        return 0;
    }

    std::shared_ptr<AbstractItem> consume(int id, int value, std::shared_ptr<SQLServer> sql_server)
    {
        if (m_posmap.find(id) != m_posmap.end())
        {
            int tmp = m_vec[m_posmap[id].first][m_posmap[id].second]->get_amount();
            if (tmp + value < 0)
                return nullptr;
            if ((m_vec[m_posmap[id].first][m_posmap[id].second]->get_amount() + value) == 0)
            {
                auto p = m_vec[m_posmap[id].first][m_posmap[id].second];
                m_vec[m_posmap[id].first][m_posmap[id].second].reset();
                m_num[m_posmap[id].first]--;
                m_posmap.erase(id);
                sql_server->query(("DELETE FROM PackageInfo WHERE item_id=" + std::to_string(id) + ";").c_str());
                return p;
            }
            m_vec[m_posmap[id].first][m_posmap[id].second]->set_amount(tmp + value);
            auto p = m_vec[m_posmap[id].first][m_posmap[id].second];
            sql_server->query(("UPDATE PackageInfo SET item_num=" + std::to_string(tmp + value) + " WHERE item_id=" + std::to_string(id) + ";").c_str());
            return p;
        }
        else
            return nullptr;
    }

    int get_num(int id)
    {
        return m_vec[m_posmap[id].first][m_posmap[id].second]->get_amount();
    }

    std::shared_ptr<AbstractItem> get_vec(int i, int j)
    {
        return m_vec[i][j];
    }

    std::unordered_map<int, std::pair<int, int>> get_map()
    {
        return m_posmap;
    }

private:
    std::vector<std::vector<std::shared_ptr<AbstractItem>>> m_vec;
    std::unordered_map<int, std::pair<int, int>> m_posmap;
    std::vector<int> m_num;
    ItemFactory m_factory;
};
#endif