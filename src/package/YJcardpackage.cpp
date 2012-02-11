#include "maneuvering2.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "standard.h"
#include "standard-equips.h"
#include "general.h"
#include "engine.h"
#include "client.h"
#include "room.h"
#include "carditem.h"

ZhangQi::ZhangQi(Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("zhang_qi");
}

void ZhangQi::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *card = room->askForCard(effect.to, "Peach,Analeptic|.|.|.|.", "zhangqi:" + effect.from->objectName());
    if(card){
        if(card->inherits("Peach"))
            room->setEmotion(effect.to, "peach");
        else if(card->inherits("Analeptic"))
            room->setEmotion(effect.to, "analeptic");
    }else{
        QString choice = room->askForChoice(effect.to, "zhang_qi", "turnover+cancel");
        if(choice == "cancel")
            room->loseHp(effect.to, 1);
        else
            effect.to->turnOver();
    }
}

YuQinGuZong::YuQinGuZong(Suit suit, int number)
    :SingleTargetTrick(suit, number, true)
{
    setObjectName("yuqinguzong");
}

bool YuQinGuZong::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void YuQinGuZong::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.to->drawCards(2);
    room->askForDiscard(effect.to, "yuqinguzong", 4, false, true);
}


Maneuvering2Package::Maneuvering2Package()
    :Package("maneuvering2")
{
    QList<Card *> cards;

    cards << new ZhangQi(Card::Club, 3)
          << new YuQinGuZong(Card::Heart, 11);


    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(Maneuvering2)
