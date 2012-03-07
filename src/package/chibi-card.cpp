#include "chibi-card.h"
#include "skill.h"
#include "standard.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"
#include "general.h"
#include "standard.h"
#include "client.h"
#include "serverplayer.h"
#include "room.h"
#include "settings.h"
#include "god.h"
#include "generaloverview.h"
#include "standard-equips.h"

// Basic Card
WindSlash::WindSlash(Suit suit, int number)
    :NatureSlash(suit, number, DamageStruct::Wind)
{
    setObjectName("wind_slash");
}

Crisp::Crisp(Card::Suit suit, int number)
    :BasicCard(suit, number)
{
    setObjectName("crisp");
    target_fixed = true;
    once = true;
}

QString Crisp::getSubtype() const{
    return "buff_card";
}

QString Crisp::getEffectPath(bool ) const{
    return Card::getEffectPath();
}

bool Crisp::IsAvailable(const Player *player){
    return !player->hasUsed("Crisp");
}

bool Crisp::isAvailable(const Player *player) const{
    return IsAvailable(player);
}

void Crisp::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    room->cardEffect(this, source, source);
}

void Crisp::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    // do animation
    QString who = effect.to->objectName();
    QString animation_str = QString("crisp:%1:%2").arg(who).arg(who);
    room->broadcastInvoke("animate", animation_str);

    LogMessage log;
    log.type = "#Crisp";
    log.from = effect.to;
    room->sendLog(log);

    room->setPlayerFlag(effect.to, "crisp");
}

// Equip Card
class BaguaSwordSkill: public WeaponSkill{
public:
    BaguaSwordSkill():WeaponSkill("bagua_sword"){
        events << SlashEffect;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!room->askForSkillInvoke(player, objectName(), data))
            return false;
        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(heart):(.*)");
        judge.good = false;
        judge.reason = objectName();
        judge.who = player;

        room->judge(judge);
        if(judge.isGood()){
            QStringList natures;
            natures << "normal_nature" << "fire_nature" << "thunder_nature" << "wind_nature";
            QString choice = room->askForChoice(player, objectName(), natures.join("+"));
            if(choice == "normal_nature")
                effect.nature = DamageStruct::Normal;
            else if(choice == "fire_nature")
                effect.nature = DamageStruct::Fire;
            else if(choice == "thunder_nature")
                effect.nature = DamageStruct::Thunder;
            else if(choice == "wind_nature")
                effect.nature = DamageStruct::Wind;

            LogMessage log;
            log.type = "#BaguaSwordLog";
            log.from = player;
            log.arg = choice;
            room->sendLog(log);

            data = QVariant::fromValue(effect);
            return false;
        }else{
            Jink *jink = new Jink(Card::NoSuit, 0);
            room->slashResult(effect, jink);
            return true;
        }
        return false;
    }
};

BaguaSword::BaguaSword(Suit suit, int number):Weapon(suit, number, 2){
    setObjectName("bagua_sword");
    skill = new BaguaSwordSkill;
}

class CatapultSkill: public WeaponSkill{
public:
    CatapultSkill():WeaponSkill("catapult"){
        events << Predamage ;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(event == Predamage){
            if(!(damage.to->getArmor() && damage.to->getArmor()->objectName() == "wall"))
                return false;
            if(!player->askForSkillInvoke(objectName(), data))
                return false;
            room->throwCard(damage.to->getArmor());
            LogMessage log;
            log.type = "#CatapultDamage";
            log.from = damage.to;
            log.to << player;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

Catapult::Catapult(Suit suit, int number)
    :Weapon(suit, number, 5)
{
    setObjectName("catapult");
    skill = new CatapultSkill;
}

class WallSkill: public ArmorSkill{
public:
    WallSkill():ArmorSkill("wall"){
        events << SlashEffected << Predamaged << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == SlashEffected){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.nature != DamageStruct::Normal){
                LogMessage log;
                log.from = player;
                log.type = "#ArmorNullify";
                log.arg = objectName();
                log.arg2 = effect.slash->objectName();
                player->getRoom()->sendLog(log);

                return true;
            }
        }else if(event == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->inherits("Flooding")){
                LogMessage log;
                log.from = player;
                log.type = "#ArmorNullify";
                log.arg = objectName();
                log.arg2 = effect.card->objectName();
                player->getRoom()->sendLog(log);

                return true;
            }
        }else if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature == DamageStruct::Normal && damage.damage > 1){
                LogMessage log;
                log.type = "#WallDamage";
                log.from = player;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(damage.damage - 1);
                player->getRoom()->sendLog(log);

                damage.damage --;
                data = QVariant::fromValue(damage);
            }
        }

        return false;
    }
};

Wall::Wall(Suit suit, int number):Armor(suit, number){
    setObjectName("wall");
    skill = new WallSkill;
}

class BlackArmorViewAsSkill: public ViewAsSkill{
public:
    BlackArmorViewAsSkill():ViewAsSkill("black_armor"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@black_armor";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int x = Self->getMark("BlackArmor");
        if(selected.length() >= x)
            return false;

        if(to_select->getFilteredCard()->isBlack())
            return false;

        if(to_select->isEquipped())
            return false;

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        int x = Self->getMark("BlackArmor");
        if(cards.length() != x)
            return NULL;

        DummyCard *card = new DummyCard;
        card->setSkillName(objectName());
        card->addSubcards(cards);
        return card;
    }
};

class BlackArmorSkill: public ArmorSkill{
public:
    BlackArmorSkill():ArmorSkill("black_armor"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        room->setPlayerMark(player, "BlackArmor", damage.damage);
        if(!player->isWounded() || player->getCards("h").length() < damage.damage)
            return false;

        CardStar card = room->askForCard(player, "@black_armor", "@black_armor");
        if(card){
            QList<int> card_ids = card->getSubcards();
            foreach(int card_id, card_ids){
                LogMessage log;
                log.type = "$DiscardCard";
                log.from = player;
                log.card_str = QString::number(card_id);

                room->sendLog(log);
            }

            LogMessage log;
            log.type = "#BlackArmorSkill";
            log.from = player;
            room->sendLog(log);

            RecoverStruct recover;
            recover.card = card;
            recover.recover = qMin(damage.damage, player->getLostHp());
            recover.who = player;
            room->recover(player, recover);

            room->setPlayerMark(player, "BlackArmor", 0);
            return true;
        }
        room->setPlayerMark(player, "BlackArmor", 0);
        return false;
    }
};

BlackArmor::BlackArmor(Suit suit, int number):Armor(suit, number)
{
    setObjectName("black_armor");
    skill = new BlackArmorSkill;
    attach_skill = true;
}

void BlackArmor::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    room->getThread()->addTriggerSkill(skill);
    room->attachSkillToPlayer(player, objectName());
}

void BlackArmor::onUninstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    room->detachSkillFromPlayer(player, objectName());
}

class BoatSkill: public TriggerSkill{
public:
    BoatSkill():TriggerSkill("boat_skill"){
        events << Predamage << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player->getDefensiveHorse()&&player->getDefensiveHorse()->inherits("Boat");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if((event == Predamage && damage.card->inherits("WindSlash")) ||
                (event == Predamaged && damage.card->inherits("FireSlash"))){
            damage.damage = damage.damage + 1;
            data = QVariant::fromValue(damage);
            LogMessage log;
            log.type = "$BoatLog";
            log.from = player;
            log.card_str = damage.card->toString();
            player->getRoom()->sendLog(log);
        }

        return false;
    }
};

Boat::Boat(Card::Suit suit, int number)
    :DefensiveHorse(suit, number)
{
    setObjectName("boat");

    boatskill = new BoatSkill;
    boatskill->setParent(this);
}

void Boat::onInstall(ServerPlayer *player) const{
    player->getRoom()->getThread()->addTriggerSkill(boatskill);
}

QString Boat::getEffectPath(bool ) const{
    return "audio/card/common/boat.ogg";
}

// Single Trick
Raid::Raid(Suit suit, int number):SingleTargetTrick(suit, number, true) {
    setObjectName("raid");
}

bool Raid::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void Raid::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *discard = NULL;
    if(!effect.to->isNude()){
        discard = room->askForCard(effect.to, ".|.|.|.|black", "@raid:"+effect.from->getGeneralName());
        if(discard){
            LogMessage log;
            log.type = "$DiscardCard";
            log.from = effect.to;
            log.card_str = discard->toString();

            room->sendLog(log);
        }
    }
    if(effect.to->isNude() || !discard){
        DamageStruct damage;
        damage.from = effect.from;
        damage.to = effect.to;
        damage.damage = 1;
        room->damage(damage);
    }
}

Steal::Steal(Card::Suit suit, int number)
    :SingleTargetTrick(suit, number, true)
{
    setObjectName("steal");
}

bool Steal::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->isKongcheng())
        return false;

    if(!Self->inMyAttackRange(to_select))
        return false;

    if(to_select == Self)
        return Self->getHandcardNum() >= 2;
    else
        return true;
}

void Steal::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(effect.to->isKongcheng())
        return;

    Card::Suit suit = room->askForSuit(effect.from, objectName());
    QString suit_str = Card::Suit2String(suit);

    int card_id = room->askForCardChosen(effect.from, effect.to, "h", objectName());
    room->showCard(effect.to, card_id);
    const Card *card = Sanguosha->getCard(card_id);

    if(card->getSuitString() == suit_str){
        DamageStruct damage;
        damage.card = this;
        damage.from = effect.from;
        damage.to = effect.to;
        damage.nature = DamageStruct::Normal;

        room->damage(damage);
    }else
        room->moveCardTo(card, effect.from, Player::Hand, true);

    if(card->isVirtualCard())
        delete card;
}

// AOE Trick
Flooding::Flooding(Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("flooding");
}

bool Flooding::isAvailable(const Player *player) const{
    return true;
}

bool Flooding::isCancelable(const CardEffectStruct &effect) const{
    QVariantList fl_list = effect.to->getRoom()->getTag("Flooding").toList();
    QList<int> card_ids;
    foreach(QVariant card_id, fl_list)
        card_ids << card_id.toInt();
    return !card_ids.isEmpty();
}

void Flooding::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    QList<ServerPlayer *> players = targets.isEmpty() ? room->getOtherPlayers(source) : targets;
    QList<int> ids = room->getNCards(players.length()), card_ids;
    card_ids = ids;
    foreach(int id, ids){
        const Card *cd = Sanguosha->getCard(id);
        if(cd->isRed())
            card_ids.removeOne(id);
    }

    room->fillAG(card_ids);

    QVariantList fl_list;
    foreach(int card_id, card_ids)
            fl_list << card_id;

    room->setTag("Flooding", fl_list);

    AOE::use(room, source, players);

    fl_list = room->getTag("Flooding").toList();

    // throw the rest cards
    foreach(QVariant card_id, fl_list){
        room->takeAG(NULL, card_id.toInt());
    }

    room->broadcastInvoke("clearAG");
}

void Flooding::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QVariantList fl_list = room->getTag("Flooding").toList();
    QList<int> card_ids;
    foreach(QVariant card_id, fl_list)
        card_ids << card_id.toInt();

    if(card_ids.isEmpty())
        return;

    int card_id = room->askForAG(effect.to, card_ids, false, objectName());
    card_ids.removeOne(card_id);

    room->takeAG(effect.to, card_id);
    fl_list.removeOne(card_id);

    room->setTag("Flooding", fl_list);

    if(!room->askForCard(effect.to, ".|.|.|.|red", "@flooding:"+effect.from->getGeneralName())){
        DamageStruct damage;
        damage.card = this;
        damage.from = effect.from;
        damage.to = effect.to;
        damage.damage = 1;
        room->damage(damage);
    }
}

// Delayed Trick
Sucker::Sucker(Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("sucker");
    target_fixed = false;

    judge.pattern = QRegExp("(.*):(club|spade):(.*)");
    judge.good = true;
    judge.reason = objectName();
}

bool Sucker::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if(!targets.isEmpty())
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    return true;
}

void Sucker::takeEffect(ServerPlayer *target) const{
    target->skip(Player::Discard);
}

//------------------------------------------------------------------------


ChibiCardPackage::ChibiCardPackage()
    :Package("ChibiCard")
{
    QList<Card *> cards;

    // spade
    cards << new Catapult(Card::Spade, 1)
            << new Wall(Card::Spade, 2)
            << new Analeptic(Card::Spade, 3)
            << new WindSlash(Card::Spade, 4)
            << new ThunderSlash(Card::Spade, 5)
            << new WindSlash(Card::Spade, 6)
            << new ThunderSlash(Card::Spade, 7)
            << new WindSlash(Card::Spade, 8)
            << new Crisp(Card::Spade, 9)
            << new Sucker(Card::Spade,10)
            << new Raid(Card::Spade, 11)
            << new IronChain(Card::Spade, 12)
            << new IronChain(Card::Spade, 13);

    // club
    cards << new BlackArmor(Card::Club, 1)
            << new Steal(Card::Club, 2)
            << new Analeptic(Card::Club, 3)
            << new Sucker(Card::Club, 4)
            << new ThunderSlash(Card::Club, 5)
            << new WindSlash(Card::Club, 6)
            << new ThunderSlash(Card::Club, 7)
            << new WindSlash(Card::Club, 8)
            << new Analeptic(Card::Club, 9)
            << new Raid(Card::Club, 10)
            << new IronChain(Card::Club, 11)
            << new Steal(Card::Club, 12)
            << new Nullification(Card::Club, 13);

    // heart
    cards << new Steal(Card::Heart, 1)
            << new IronChain(Card::Heart, 2)
            << new Wall(Card::Heart, 3)
            << new WindSlash(Card::Heart, 4)
            << new Peach(Card::Heart, 5)
            << new Crisp(Card::Heart, 6)
            << new FireSlash(Card::Heart, 7)
            << new Jink(Card::Heart, 8)
            << new Jink(Card::Heart, 9)
            << new FireSlash(Card::Heart, 10)
            << new Jink(Card::Heart, 11)
            << new Jink(Card::Heart, 12)
            << new Nullification(Card::Heart, 13);

    // diamond
    cards << new BaguaSword(Card::Diamond, 1)
            << new Peach(Card::Diamond, 2)
            << new Peach(Card::Diamond, 3)
            << new WindSlash(Card::Diamond, 4)
            << new FireSlash(Card::Diamond, 5)
            << new Jink(Card::Diamond, 6)
            << new Jink(Card::Diamond, 7)
            << new Jink(Card::Diamond, 8)
            << new Crisp(Card::Diamond, 9)
            << new Jink(Card::Diamond, 10)
            << new Jink(Card::Diamond, 11)
            << new Flooding(Card::Diamond, 12)
            << new Boat(Card::Diamond, 13);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;

    skills << new BlackArmorViewAsSkill;
}

ADD_PACKAGE(ChibiCard)
