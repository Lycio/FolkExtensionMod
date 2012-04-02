#include "dishacardpackage.h"
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
PunctureSlash::PunctureSlash(Card::Suit suit, int number)
    :Slash(suit, number)
{
    setObjectName("puncture_slash");
}

BloodSlash::BloodSlash(Card::Suit suit, int number)
    :Slash(suit, number)
{
    setObjectName("blood_slash");
}

PoisonPeach::PoisonPeach(Suit suit, int number)
    :Peach(suit, number)
{
    setObjectName("poison_peach");
    target_fixed = true;
}

QString PoisonPeach::getSubtype() const{
    return "disgusting_card";
}

QString PoisonPeach::getEffectPath(bool is_male) const{
    if(is_male)
        return "audio/card/male/poison_peach.ogg";
    else
        return "audio/card/female/poison_peach.ogg";
}

void PoisonPeach::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    // do animation
    room->broadcastInvoke("animate", QString("peach:%1:%2")
                          .arg(effect.to->objectName())
                          .arg(effect.to->objectName()));

    // damage
    DamageStruct damage;
    damage.card = this;
    damage.from = effect.to;
    damage.to = effect.to;
    damage.damage = 1;
    damage.nature = DamageStruct::Normal;

    room->damage(damage);
}

bool PoisonPeach::isAvailable(const Player *player) const{
    return true;
}

// Equip Card
class YitianJianSkill: public WeaponSkill{
public:
    YitianJianSkill():WeaponSkill("yitian_jian"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *weaponOwner = NULL;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->getWeapon() && p->getWeapon()->inherits("YitianJian"))
                weaponOwner = p;
        }
        if(weaponOwner == NULL)
            return false;
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.to->isAlive())
            return false;
        if(!weaponOwner->inMyAttackRange(damage.to))
            return false;
        int slashNum = 0;
        foreach(const Card *cd, weaponOwner->getHandcards()){
            if(cd->inherits("Slash"))
                slashNum++;
        }
        if(slashNum < 2)
            return false;
        if(weaponOwner->askForSkillInvoke(objectName(), data)){
            if(room->askForCard(weaponOwner, "slash", "@yitianjian_first") && room->askForCard(weaponOwner, "slash", "@yitianjian_second")){
                weaponOwner->addMark("yitian_canuse");
            }
            if(weaponOwner->getMark("yitian_canuse")){
                room->setTag("Yitian_jianTarget", QVariant::fromValue(damage.to));
                if(room->askForChoice(weaponOwner, objectName(), "todamage+torecover") == "torecover"){
                    RecoverStruct recover;
                    recover.who = weaponOwner;
                    recover.recover = 1;
                    room->recover(damage.to, recover);
                }else{
                    DamageStruct toDamage;
                    toDamage.card = NULL;
                    toDamage.chain = false;
                    toDamage.damage = 1;
                    toDamage.nature = damage.nature;
                    toDamage.from = damage.from;
                    toDamage.to = damage.to;
                    room->damage(toDamage);
                }
                weaponOwner->removeMark("yitian_canuse");
                room->removeTag("Yitian_jianTarget");
                return false;
            }
        }
        return false;
    }
};

YitianJian::YitianJian(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("yitian_jian");
    skill = new YitianJianSkill;
}

class QixingBladeSkill: public WeaponSkill{
public:
    QixingBladeSkill():WeaponSkill("qixing_blade"){
        events << SlashHit;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasWeapon(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = player->getRoom();
        if(player->isNude())
            return false;
        if(player->getCards("he").length() < 2)
            return false;
        if(player->askForSkillInvoke(objectName(), data)){            
            int i;
            for(i = 0; i < 2; i++){
                int card_id = room->askForCardChosen(player, player, "he", objectName());
                const Card *card = Sanguosha->getCard(card_id);
                room->moveCardTo(card, effect.to, Player::Hand, room->getCardPlace(card->getEffectiveId()) == Player::Hand ? false : true);
            }
            effect.to->turnOver();
            return true;
        }

        return false;
    }
};

QixingBlade::QixingBlade(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("qixing_blade");
    skill = new QixingBladeSkill;
}

class LuofengBowSkill: public WeaponSkill{
public:
    LuofengBowSkill():WeaponSkill("luofeng_bow"){
        events << SlashProceed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasWeapon(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = player->getRoom();
        if(effect.to->getHandcardNum() > player->getHp()){
            room->slashResult(effect, NULL);
            return true;
        }
        return false;
    }
};

LuofengBow::LuofengBow(Suit suit, int number)
    :Weapon(suit, number, 1)
{
    setObjectName("luofeng_bow");
    skill = new LuofengBowSkill;
}

class JiaSuoSkill: public WeaponSkill{
public:
    JiaSuoSkill():WeaponSkill("jiasuo"){
        events << CardUsed << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(!player->hasFlag("dying")){
                if(use.card->inherits("Peach") || use.card->inherits("Analeptic")){
                    player->getRoom()->throwCard(use.card);
                    return true;
                }
            }
            return false;
        }else if(event == PhaseChange && player->getPhase() == Player::Start){
            if(player->getArmor() && player->getArmor()->inherits("JiaoLiao")){
                LogMessage log;
                log.type = "#JiaSuoLog";
                log.from = player;
                player->getRoom()->sendLog(log);
                player->skip(Player::Play);
                player->setMark("skippedPlaying", 1);
                return false;
            }
        }
        return false;
    }
};

JiaSuo::JiaSuo(Suit suit, int number)
    :Weapon(suit, number, 1)
{
    setObjectName("jiasuo");
    skill = new JiaSuoSkill;

    target_fixed = false;
}

bool JiaSuo::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void JiaSuo::onUse(Room *room, const CardUseStruct &card_use) const{
    Card::onUse(room, card_use);
}

class JiaoLiaoSkill: public ArmorSkill{
public:
    JiaoLiaoSkill():ArmorSkill("jiaoliao"){
        events << SlashEffected << CardAsked << PhaseChange;
    }

    const Card *askForDoubleJink(ServerPlayer *player, const QString &reason) const{
        Room *room = player->getRoom();

        const Card *first_jink = NULL, *second_jink = NULL;
        first_jink = room->askForCard(player, "Jink", QString("@%1-jink-1").arg(reason));
        if(first_jink)
            second_jink = room->askForCard(player, "Jink", QString("@%1-jink-2").arg(reason));

        Card *jink = NULL;
        if(first_jink && second_jink){
            jink = new DummyCard;
            jink->addSubcard(first_jink);
            jink->addSubcard(second_jink);
            jink->setSkillName(reason);
        }

        return jink;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == SlashEffected){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            room->slashResult(effect, askForDoubleJink(player, objectName()));
            return true;
        }else if(event == CardAsked){
            QString asked = data.toString();
            if(asked == "jink"){
                const Card *doublejink = askForDoubleJink(player, objectName());
                if(doublejink){
                    Jink *jink = new Jink(Card::NoSuit, 0);
                    //room->setEmotion(player, "jink");
                    room->provide(jink);
                    return true;
                }else
                    room->provide(NULL);
                return false;
            }
        }else if(event == PhaseChange && player->getPhase() == Player::Finish && player->getMark("skippedPlaying") == 1){
            player->setMark("skippedPlaying", 0);
            room->throwCard(player->getWeapon());
            room->throwCard(player->getArmor());
            return false;
        }
        return false;
    }
};

JiaoLiao::JiaoLiao(Suit suit, int number)
    :Armor(suit, number)
{
    setObjectName("jiaoliao");
    skill = new JiaoLiaoSkill;

    target_fixed = false;
}

bool JiaoLiao::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->getArmor();
}

void JiaoLiao::onUse(Room *room, const CardUseStruct &card_use) const{
    Card::onUse(room, card_use);
}
/*
void JiaoLiao::onUninstall(ServerPlayer *player) const{
    player->removeMark("SkippedPlaying");
}
*/
// Single Trick
SuddenStrike::SuddenStrike(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("sudden_strike");
    target_fixed = true;
}

void SuddenStrike::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(!room->askForCard(effect.to, "jink", "@sudden_strike-jink:"+effect.from->getGeneralName())){
        room->loseHp(effect.to, 1);
        if(effect.from->hasSkill("yanbaoshang"))
            if(effect.from->askForSkillInvoke("yanbaoshang"))
                effect.from->drawCards(1);
    }else
        return;
}

bool SuddenStrike::isAvailable(const Player *) const{
    return false;
}

Cover::Cover(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("cover");
    target_fixed = true;
}

void Cover::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *cover_card = room->getTag("CoverCard").value<CardStar>();
    LogMessage log;
    log.type = "$CoverLog";
    log.from = effect.from;
    log.to << effect.to;
    log.card_str = cover_card->toString();
    room->sendLog(log);
    room->setTag("CoverFrom", QVariant::fromValue(effect.from));
}

bool Cover::isAvailable(const Player *) const{
    return false;
}

Rebound::Rebound(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("rebound");
    target_fixed = true;
}

void Rebound::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    LogMessage log;
    log.type = "#ReboundLog";
    log.from = effect.from;
    log.to << effect.to;
    room->sendLog(log);

    DamageStruct damage = room->getTag("ReboundStruct").value<DamageStruct>();

    DamageStruct rebound;
    rebound.card = damage.card;
    rebound.chain = damage.chain;
    rebound.damage = damage.damage;
    rebound.from = damage.to;
    rebound.nature = damage.nature;
    rebound.to = damage.from;
    room->damage(rebound);

    room->setTag("ReboundEffected", true);
}

bool Rebound::isAvailable(const Player *) const{
    return false;
}

Rob::Rob(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("rob");
    target_fixed = true;
}

void Rob::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(effect.to->isKongcheng())
        return;

    room->moveCardTo(Sanguosha->getCard(room->askForCardChosen(effect.from, effect.to, "h", "rob")), effect.from, Player::Hand, false);
}

bool Rob::isAvailable(const Player *) const{
    return false;
}

// AOE Trick
IrresistibleForce::IrresistibleForce(Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("irresistible_force");
}

bool IrresistibleForce::isAvailable(const Player *player) const{
    foreach(const Player *p, player->getSiblings()){
        if(p->getEquips().length() != 0 && p->isAlive())
            return true;
    }

    return false;
}

bool IrresistibleForce::isCancelable(const CardEffectStruct &effect) const{
    return effect.to->getEquips().length() != 0;
}

void IrresistibleForce::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    if(effect.to->getEquips().length() != 0)
        room->loseHp(effect.to, 1);
}

// Delayed Trick
Plague::Plague(Suit suit, int number):Disaster(suit, number){
    setObjectName("plague");

    judge.pattern = QRegExp("(.*):(club|spade|heart):(.*)");
    judge.good = false;
    judge.reason = objectName();
}

void Plague::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    LogMessage log;
    log.from = effect.to;
    log.type = "#DelayedTrick";
    log.arg = effect.card->objectName();
    room->sendLog(log);

    JudgeStruct judge_struct = judge;
    judge_struct.who = effect.to;
    room->judge(judge_struct);

    if(judge_struct.isBad()){
        takeEffect(effect.to);
    }else
        room->throwCard(this);
}

void Plague::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();
    room->loseHp(target, 1);
    ServerPlayer *next = target->getNextAlive();
    room->moveCardTo(this, next, Player::Judging, true);
}

//------------------------------------------------------------------------


DishaCardPackage::DishaCardPackage()
    :Package("DishaCard")
{
    QList<Card *> cards;

    // spade
    cards << new YitianJian(Card::Spade, 1)
          << new Plague(Card::Spade, 2)
          << new Rebound(Card::Spade, 3)
          << new SuddenStrike(Card::Spade, 4)
          << new SuddenStrike(Card::Spade, 5)
          << new Cover(Card::Spade, 6)
          << new PoisonPeach(Card::Spade, 7)
          << new PunctureSlash(Card::Spade, 8)
          << new PunctureSlash(Card::Spade, 9)
          << new PunctureSlash(Card::Spade,10)
          << new Plague(Card::Spade, 11)
          << new IrresistibleForce(Card::Spade, 12)
          << new Nullification(Card::Spade, 13);

    OffensiveHorse *yuzhaoyeshizi = new OffensiveHorse(Card::Club, 13);
    yuzhaoyeshizi->setObjectName("yuzhaoyeshizi");

    // club
    cards << new QixingBlade(Card::Club, 1)
          << new JiaSuo(Card::Club, 2)
          << new JiaoLiao(Card::Club, 3)
          << new Rob(Card::Club, 4)
          << new Rob(Card::Club, 5)
          << new Cover(Card::Club, 6)
          << new PoisonPeach(Card::Club, 7)
          << new PunctureSlash(Card::Club, 8)
          << new PoisonPeach(Card::Club, 9)
          << new PunctureSlash(Card::Club, 10)
          << new JiaoLiao(Card::Club, 11)
          << new LuofengBow(Card::Club, 12);

    cards << yuzhaoyeshizi;

    // heart
    cards << new Rebound(Card::Heart, 1)
          << new Rebound(Card::Heart, 2)
          << new Jink(Card::Heart, 3)
          << new Peach(Card::Heart, 4)
          << new Cover(Card::Heart, 5)
          << new Jink(Card::Heart, 6)
          << new BloodSlash(Card::Heart, 7)
          << new BloodSlash(Card::Heart, 8)
          << new BloodSlash(Card::Heart, 9)
          << new Cover(Card::Heart, 10)
          << new Peach(Card::Heart, 11)
          << new Peach(Card::Heart, 12)
          << new Rob(Card::Heart, 13);

    // diamond
    cards << new SuddenStrike(Card::Diamond, 1)
          << new Jink(Card::Diamond, 2)
          << new Jink(Card::Diamond, 3)
          << new Jink(Card::Diamond, 4)
          << new SuddenStrike(Card::Diamond, 5)
          << new Cover(Card::Diamond, 6)
          << new BloodSlash(Card::Diamond, 7)
          << new BloodSlash(Card::Diamond, 8)
          << new Jink(Card::Diamond, 9)
          << new Jink(Card::Diamond, 10)
          << new Cover(Card::Diamond, 11)
          << new Rob(Card::Diamond, 12)
          << new Nullification(Card::Diamond, 13);

    foreach(Card *card, cards)
        card->setParent(this);

    addMetaObject<PunctureSlash>();
    addMetaObject<BloodSlash>();

    type = CardPack;
}

ADD_PACKAGE(DishaCard)
