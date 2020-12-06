#ifndef ITEMFAC_H
#define ITEMFAC_H
#include "Item.h"
#include "Money.h"
#include "Equip.h"
#include "Consume.h"

class ItemFactory
{
public:
    ItemFactory() {}
    std::shared_ptr<AbstractItem> create(ItemInfo info, int value)
    {
        if (info.mtype == EltemType::eMoney)
            return std::make_shared<Money>(info, value);
        else if (info.mtype == EltemType::eEQUIP)
            return std::make_shared<Equip>(info, value);
        else if (info.mtype == EltemType::eCONSUME)
            return std::make_shared<Consume>(info, value);
        else
            return nullptr;
    }
};
#endif