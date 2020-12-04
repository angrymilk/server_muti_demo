#include "Player.h"

Player::~Player()
{
}

int Player::uin()
{
    return m_uin;
}

void Player::set_uin(int id)
{
    m_uin = id;
}

int Player::get_num(int id)
{
    return m_package->get_num(id);
}

int Player::add(ItemInfo info, int pos, int value, bool to_package, std::shared_ptr<SQLServer> sql_server)
{
    if (!to_package)
    {
        //同样道具目前只能个数为1
        if (value != 1 || info.mtype != EltemType::eEQUIP || m_in_use.find(info.id) != m_in_use.end())
            return -1;
        m_in_use[info.id] = std::make_shared<Equip>(info, value);
        m_hp += m_in_use[info.id]->get_attribute(EltemModuleType::eltem_Module_Base, EltemAttributeType::eltem_Attribute_HP);
        m_attack += m_in_use[info.id]->get_attribute(EltemModuleType::eltem_Module_Base, EltemAttributeType::eltem_Attribute_Attack);
        sql_server->query(("UPDATE PlayerInfo SET hp=" + std::to_string(m_hp) + ",attack=" + std::to_string(m_attack) + "WHERE player_id=" + std::to_string(m_uin) + ";").c_str());
        sql_server->query(("INSERT INTO UseInfo ( player_id, item_id,item_num ) VALUES ( " + std::to_string(m_uin) + "," + std::to_string(info.id) + "," + std::to_string(1) + ");").c_str());
        return 0;
    }
    return m_package->add(info, pos, value, sql_server);
}

int Player::consume(int id, EltemType type, int value, bool to_package, bool inuse, std::shared_ptr<SQLServer> sql_server)
{
    if (!to_package) //从非背包中丢弃道具
    {
        //同样道具目前只能个数为1
        if (value != -1 || type != EltemType::eEQUIP || m_in_use.find(id) == m_in_use.end())
            return -1;
        m_hp -= m_in_use[id]->get_attribute(EltemModuleType::eltem_Module_Base, EltemAttributeType::eltem_Attribute_HP);
        m_attack -= m_in_use[id]->get_attribute(EltemModuleType::eltem_Module_Base, EltemAttributeType::eltem_Attribute_Attack);
        m_in_use.erase(id);
        sql_server->query(("UPDATE PlayerInfo SET hp=" + std::to_string(m_hp) + ",attack=" + std::to_string(m_attack) + "WHERE player_id=" + std::to_string(m_uin) + ";").c_str());

        return 0;
    }

    if (inuse) //从背包中移动道具到使用中的物品栏中
    {
        printf("从背包中移动道具到使用中的物品栏中\n");
        if (value != -1 || m_in_use.find(id) != m_in_use.end())
            return -1;
        m_in_use[id] = m_package->consume(id, value, sql_server);
        m_hp += m_in_use[id]->get_attribute(EltemModuleType::eltem_Module_Base, EltemAttributeType::eltem_Attribute_HP);
        m_attack += m_in_use[id]->get_attribute(EltemModuleType::eltem_Module_Base, EltemAttributeType::eltem_Attribute_Attack);
        sql_server->query(("UPDATE PlayerInfo SET hp=" + std::to_string(m_hp) + ",attack=" + std::to_string(m_attack) + "WHERE player_id=" + std::to_string(m_uin) + ";").c_str());
        sql_server->query(("DELETE FROM UseInfo WHERE item_id=" + std::to_string(id) + ";").c_str());
        return 0;
    }
    auto p = m_package->consume(id, value, sql_server);
    if (!p)
        return -1;
    if (p->get_eltem_type() == EltemType::eCONSUME)
    {
        m_hp += abs(value) * (p->get_attribute(EltemModuleType::eltem_Module_Base, EltemAttributeType::eltem_Attribute_HP));
        m_attack += abs(value) * (p->get_attribute(EltemModuleType::eltem_Module_Base, EltemAttributeType::eltem_Attribute_Attack));
        sql_server->query(("UPDATE PlayerInfo SET hp=" + std::to_string(m_hp) + ",attack=" + std::to_string(m_attack) + "WHERE player_id=" + std::to_string(m_uin) + ";").c_str());
    }
    return 0;
}

int Player::get_attack()
{
    return m_attack;
}

int Player::get_hp()
{
    return m_hp;
}

std::shared_ptr<Package> Player::get_package()
{
    return m_package;
}