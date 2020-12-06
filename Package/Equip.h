#ifndef EQIUP_H
#define EQIUP_H
#include "Item.h"
class Equip : public Item
{
public:
    Equip(ItemInfo info, int value) : Item(info)
    {
        set_amount(value);
    }

    int get_uid()
    {
        return Item::get_uid();
    }

    void set_uid(int id)
    {
        Item::set_uid(id);
    }

    void set_eltem_type(EltemType type)
    {
        Item::set_eltem_type(type);
    }

    EltemType get_eltem_type()
    {
        return Item::get_eltem_type();
    }

    int get_amount()
    {
        return Item::get_amount();
    }

    void set_amount(int num)
    {
        Item::set_amount(num);
    }

    void set_attribute(EltemModuleType mtype, EltemAttributeType type, int value)
    {
        Item::set_attribute(mtype, type, value);
    }

    int get_attribute(EltemModuleType mtype, EltemAttributeType type)
    {
        return Item::get_attribute(mtype, type);
    }
};
#endif