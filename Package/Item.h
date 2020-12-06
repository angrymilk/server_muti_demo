#ifndef ITEM_H
#define ITEM_H
#include "ItemType.h"
#include <unordered_map>
#include <memory>
class ItemInterface
{
public:
    virtual EltemType get_eltem_type() = 0;
    virtual void set_eltem_type(EltemType type) = 0;
    virtual void set_attribute(EltemModuleType mtype, EltemAttributeType type, int value) = 0;
    virtual int get_attribute(EltemModuleType mtype, EltemAttributeType type) = 0;
    virtual int get_uid() = 0;
    virtual void set_uid(int id) = 0;
    virtual void set_amount(int num) = 0;
    virtual int get_amount() = 0;
};

class AbstractItem : public ItemInterface
{
public:
    AbstractItem(EltemType eltem_type, int id) : m_eltem_type(eltem_type),
                                                 m_count(0),
                                                 m_save(true),
                                                 uid(id)
    {
    }

    EltemType get_eltem_type();
    void set_eltem_type(EltemType type);
    void set_attribute(EltemModuleType mtype, EltemAttributeType type, int value);
    int get_attribute(EltemModuleType mtype, EltemAttributeType type);
    int get_uid();
    void set_uid(int id);
    void set_amount(int num);
    int get_amount();
    //扩展标记位
    int m_flag_bit;

private:
    bool m_save;
    int m_count;
    int uid;
    EltemType m_eltem_type;
};

class ItemAttribute;
class Item : public AbstractItem, std::enable_shared_from_this<Item>
{
public:
    Item(ItemInfo info) : AbstractItem(info.mtype, info.id)
    {

        for (int i = 0; i < info.mmotype.size(); i++)
        {
            m_map[info.mmotype[i]] = std::make_shared<ItemAttribute>(this, info.mattrtype[i], info.value[i]);
        }
    }

    EltemType get_eltem_type();
    void set_eltem_type(EltemType type);
    void set_attribute(EltemModuleType mtype, EltemAttributeType type, int value);
    int get_attribute(EltemModuleType mtype, EltemAttributeType type);
    void set_amount(int num);
    int get_amount();
    int get_uid();
    void set_uid(int id);
    //扩展标记位
    int m_flag_bit;

private:
    std::unordered_map<EltemModuleType, std::shared_ptr<ItemAttribute>> m_map;
};

class ItemAttribute
{
public:
    ItemAttribute(Item *point, std::vector<EltemAttributeType> info, std::vector<int> values) : m_item(point)
    {
        for (int i = 0; i < info.size(); i++)
        {
            m_attribute_map[info[i]] = values[i];
        }
    }

    void set_attribute(EltemAttributeType type, int value);
    int get_attribute(EltemAttributeType type);

private:
    std::unordered_map<EltemAttributeType, int> m_attribute_map;
    Item *m_item;
};
#endif