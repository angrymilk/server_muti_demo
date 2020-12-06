#ifndef MONEY_H
#define MONEY_H
#include "Item.h"

class Money : public AbstractItem
{
public:
    Money(ItemInfo info, int value) : AbstractItem(info.mtype, info.id)
    {
        set_amount(value);
    }

    int get_uid()
    {
        return AbstractItem::get_uid();
    }

    void set_uid(int id)
    {
        AbstractItem::set_uid(id);
    }

    void set_eltem_type(EltemType type)
    {
        AbstractItem::set_eltem_type(type);
    }

    EltemType get_eltem_type()
    {
        return AbstractItem::get_eltem_type();
    }

    int get_amount()
    {
        return AbstractItem::get_amount();
    }

    void set_amount(int num)
    {
        AbstractItem::set_amount(num);
    }
};
#endif