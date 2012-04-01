#include "hide-package.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"

class AdouMarkSkill: public ArmorSkill{
public:
    AdouMarkSkill():ArmorSkill("adou_mark"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.damage > 1){
            LogMessage log;
            log.type = "#SilverLion";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = objectName();
            player->getRoom()->sendLog(log);

            damage.damage = 1;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

AdouMark::AdouMark(Suit suit, int number):Armor(suit, number){
    setObjectName("adou_mark");
    skill = new AdouMarkSkill;
}

void AdouMark::onUninstall(ServerPlayer *player) const{
    if(player->isAlive() && player->getMark("qinggang") == 0){
        RecoverStruct recover;
        recover.card = this;
        player->getRoom()->recover(player, recover);
    }
}

HidePackage::HidePackage()
    :Package("hide")
{
    QList<Card *> cards;

    cards << new AdouMark(Card::Club, 1);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(Hide)
