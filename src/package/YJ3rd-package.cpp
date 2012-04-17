#include "YJ3rd-package.h"
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

YjJubingCard::YjJubingCard(){
    target_fixed = true;
}

void YjJubingCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(heart):(.*)");
    judge.reason = "yjjubing";
    judge.who = source;
    judge.good = false;
    room->judge(judge);

    if(judge.isGood()){
        QList<int> discarded_pile = room->getDiscardPile(), equips_id;
        foreach(int card_id, discarded_pile){
            const Card *card = Sanguosha->getCard(card_id);
            if(card->inherits("EquipCard"))
                equips_id << card_id;
        }
        if(equips_id.isEmpty())
            return ;
        room->fillAG(equips_id, source);
        int equip_id = room->askForAG(source, equips_id, true, "yjjubing");
        source->invoke("clearAG");
        if(equip_id != -1)
            room->moveCardTo(Sanguosha->getCard(equip_id), source, Player::Hand, true);
    }
}

class YjJubing: public ZeroCardViewAsSkill{
public:
    YjJubing():ZeroCardViewAsSkill("yjjubing"){
    }

    virtual const Card *viewAs() const{
        return new YjJubingCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("YjJubingCard") ;
    }
};

class YjZhufang: public PhaseChangeSkill{
public:
    YjZhufang():PhaseChangeSkill("yjzhufang"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("yjzhufang") == 0
                && target->getPhase() == Player::Start
                && target->getEquips().length() >= 2;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#YjZhufangWake";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);

        //room->broadcastInvoke("animate", "lightbox:$yjzhufang:5000");
        //room->getThread()->delay(5000);

        if(room->askForChoice(player, objectName(), "recover+draw") == "recover"){
            RecoverStruct recover;
            recover.who = player;
            room->recover(player, recover);
        }else
            room->drawCards(player, 2);

        room->setPlayerMark(player, "yjzhufang", 1);
        room->acquireSkill(player, "yjyoujin");

        room->loseMaxHp(player);

        return false;
    }
};

YjYoujinCard::YjYoujinCard(){
    will_throw = false;
}

bool YjYoujinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return to_select != Self && targets.isEmpty();
}

void YjYoujinCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);
    effect.to->getRoom()->setTag("YjYoujinTarget", QVariant::fromValue(effect.to));
}

class YjYoujinViewAsSkill:public OneCardViewAsSkill{
public:
    YjYoujinViewAsSkill():OneCardViewAsSkill("yjyoujin"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("YjYoujinCard") ;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->inherits("EquipCard") || card->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new YjYoujinCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class YjYoujin: public TriggerSkill{
public:
    YjYoujin():TriggerSkill("yjyoujin"){
        events << PhaseChange ;
        view_as_skill = new YjYoujinViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        PlayerStar target = room->getTag("YjYoujinTarget").value<PlayerStar>();
        if(!target)
            return false;
        if(player->getPhase() == Player::Finish){
            room->removeTag("YjYoujinTarget");
            LogMessage log;
            log.type = "#YjYoujin";
            log.from = target;
            room->sendLog(log);

            room->setPlayerMark(target, objectName(), 1);
            QList<Player::Phase> play_phase ;
            play_phase << Player::Play ;
            target->play(play_phase);

            if(target->getMark(objectName()) == 1)
                target->turnOver();

            room->setPlayerMark(target, objectName(), 0);
        }

        return false;
    }
};

class YjYoujinPlay: public TriggerSkill{
public:
    YjYoujinPlay():TriggerSkill("#yjyoujin"){
        events << CardUsed ;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return ! target->hasSkill("yjyoujin");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("BasicCard") && player->getMark("yjyoujin") == 1 && player->getPhase() == Player::Play)
            player->getRoom()->setPlayerMark(player, "yjyoujin", 0);

        return false;
    }
};

static bool CompareByNumber(const Card *card1, const Card *card2){
    return card1->getNumber() > card2->getNumber();
}

YjRangliCard::YjRangliCard(){
    target_fixed = true;
}

void YjRangliCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->drawCards(source->getLostHp());
    QList<const Card *> handcards = source->getHandcards();
    foreach(const Card *cd, handcards)
        room->showCard(source, cd->getEffectiveId());
    int n = qAbs((source->getHandcardNum() + 1) / 2);
    qSort(handcards.begin(), handcards.end(), CompareByNumber);
    DummyCard *dummy_card = new DummyCard;
    int i;
    for(i=0; i<n; i++){
        dummy_card->addSubcard(handcards.at(i));
    }
    ServerPlayer *to = room->askForPlayerChosen(source, room->getOtherPlayers(source), "yjrangli");
    room->moveCardTo(dummy_card, to, Player::Hand, true);
    if(n >= 3){
        RecoverStruct rec;
        rec.recover = 1;
        room->recover(source, rec);
    }
}

class YjRangli: public ZeroCardViewAsSkill{
public:
    YjRangli():ZeroCardViewAsSkill("yjrangli"){
    }

    virtual const Card *viewAs() const{
        return new YjRangliCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("YjRangliCard") && player->isWounded();
    }
};

class YjCibian:public MasochismSkill{
public:
    YjCibian():MasochismSkill("yjcibian"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(!damage.from || damage.from->isKongcheng() || !room->askForSkillInvoke(player, objectName()))
            return;

        const Card *card_1 = room->askForCardShow(damage.from, player, objectName());
        room->showCard(damage.from, card_1->getEffectiveId());

        const Card *card_2 = Sanguosha->getCard(room->drawCard());
        room->moveCardTo(card_2, NULL, Player::DiscardedPile, true);
        LogMessage log;
        log.type = "$YjCibian_show";
        log.from = player ;
        log.card_str = card_2->toString();
        room->sendLog(log);

        bool win = card_1->getNumber() > card_2->getNumber();
        log.card_str.clear();
        log.type = "#YjCibian";
        log.from = damage.from;
        log.arg = "DrawPile";
        log.arg2 = win ? "win" : "lose" ;
        room->sendLog(log);

        if(!win){
            RecoverStruct rec;
            rec.recover = 1;
            room->recover(player, rec);
        }
    }
};

class YjMingmin:public MasochismSkill{
public:
    YjMingmin():MasochismSkill("yjmingmin"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(!damage.from || !room->askForSkillInvoke(player, objectName()))
            return;

        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(.*):(.*)");
        judge.reason = objectName();
        judge.who = player;
        room->judge(judge);

        QString color = judge.card->getColorString();
        QString suit = judge.card->getSuitString();
        room->setTag("MingminJudge", QVariant(color + "+" + suit));
        const Card *card = room->askForCard(player, ".|.|.|hand|" + color, QString("@yjmingmin:::%1:%2").arg(suit).arg(color), QVariant::fromValue(damage));
        if(card){
            LogMessage log;
            log.type = "$ResponseCard";
            log.from = player;
            log.card_str = card->toString();
            room->sendLog(log);

            if(card->getColorString() == color)
                player->drawCards(1);
            if(card->getSuitString() == suit && !damage.from->isNude()){
                int id = room->askForCardChosen(player, damage.from, "he", objectName());
                room->throwCard(id);

                LogMessage log;
                log.type = "$ToDiscardCard";
                log.from = player;
                log.to << damage.from;
                log.card_str = Sanguosha->getCard(id)->toString();
                room->sendLog(log);
            }
        }
        room->removeTag("MingminJudge");
    }
};

class YjPianquan: public TriggerSkill{
public:
    YjPianquan():TriggerSkill("yjpianquan"){
        events << PhaseChange;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *owner = room->findPlayerBySkillName(objectName());
        if(!owner || player->getPhase() != Player::Judge ||
                player->getJudgingArea().isEmpty() ||
                !owner->askForSkillInvoke(objectName()))
            return false;
        ServerPlayer *to = room->askForPlayerChosen(owner, room->getOtherPlayers(player), objectName());
        to->drawCards(1);
        return false;
    }
};

class YjRenxin: public TriggerSkill{
public:
    YjRenxin():TriggerSkill("yjrenxin$"){
        events << HpLost << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return ! target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *owner = room->findPlayerBySkillName(objectName());
        if(!owner || player->getKingdom() != "wei" || player->isDead() || !player->askForSkillInvoke(objectName()))
            return false;
        room->setTag("RenxinTarget", QVariant::fromValue(player));
        if(room->askForChoice(owner, objectName(), "draw+getacard") == "getacard"){
            int card_id = room->askForCardChosen(owner, player, "hej", objectName());
            owner->obtainCard(Sanguosha->getCard(card_id));
        }else
            owner->drawCards(1);
        room->removeTag("RenxinTarget");
        return false;
    }
};

class YjNaman: public TriggerSkill{
public:
    YjNaman():TriggerSkill("yjnaman"){
        events << CardUsed << CardFinished << CardResponsed << PhaseChange;
        frequency = Frequent ;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *owner = room->findPlayerBySkillName("yjnaman");
        if(!owner)
            return false;        
        QList<int> savages = owner->getPile("savage");
        if(savages.length() >= 5)
            return false;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->inherits("SavageAssault"))
                room->setTag("SavageAssaultProceed", QVariant(true));
        }else if(event == CardFinished){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->inherits("SavageAssault"))
                room->removeTag("SavageAssaultProceed");
        }else if(event == CardResponsed){
            if(!room->getTag("SavageAssaultProceed").toBool())
                return false;
            CardStar card = data.value<CardStar>();
            if(owner->askForSkillInvoke("yjnaman"))
                owner->addToPile("savage", card, true);
        }else if(event == PhaseChange && owner->getPhase() == Player::Draw){
            if(owner->isWounded() && owner->askForSkillInvoke("yjnaman")){
                int i;
                for(i=0; i<qMin(owner->getLostHp(), 5 - savages.length()); i++){
                    const Card *cd = Sanguosha->getCard(room->drawCard());
                    room->moveCardTo(cd, NULL, Player::Special, true);
                    owner->addToPile("savage", cd, true);
                    room->getThread()->delay();
                }
            }
        }
        return false;
    }
};

YjYinjianCard::YjYinjianCard(){

}

bool YjYinjianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *player) const{
    if(!Self->hasFlag("out_range"))
        return targets.isEmpty();
    else
        return Self->inMyAttackRange(to_select) && targets.isEmpty();
}

void YjYinjianCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QList<int> savages = effect.from->getPile("savage");
    room->fillAG(savages, effect.from);
    int savage = room->askForAG(effect.from, savages, false, "yjyinjian");
    effect.from->invoke("clearAG");
    if(savage != -1)
        room->throwCard(savage);
    if(effect.from->inMyAttackRange(effect.to))
        effect.to->drawCards(1);
    else{
        room->setPlayerFlag(effect.from, "out_range");
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("yjyinjian");
        slash->addSubcard(savage);
        if(effect.to->getArmor() && effect.to->getArmor()->inherits("Vine"))
            effect.to->addMark("qinggang");
        room->cardEffect(slash, effect.from, effect.to);
        if(effect.to->getMark("qinggang") > 0)
            effect.to->removeMark("qinggang");
    }
}

class YjYinjian: public ZeroCardViewAsSkill{
public:
    YjYinjian():ZeroCardViewAsSkill("yjyinjian"){
    }

    virtual const Card *viewAs() const{
        return new YjYinjianCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->getPile("savage").isEmpty();
    }
};

class YjZhiman: public ProhibitSkill{
public:
    YjZhiman():ProhibitSkill("yjzhiman"){

    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("SavageAssault");
    }
};

YJ3rdPackage::YJ3rdPackage()
    :Package("YJ3rd")
{
    General *yjguohuai = new General(this, "yjguohuai", "wei");
    yjguohuai->addSkill(new YjJubing);
    yjguohuai->addSkill(new YjZhufang);
    addMetaObject<YjJubingCard>();
    addMetaObject<YjYoujinCard>();

    General *yjkongrong = new General(this, "yjkongrong", "wei", 3);
    yjkongrong->addSkill(new YjRangli);
    yjkongrong->addSkill(new YjCibian);
    addMetaObject<YjRangliCard>();

    General *yjcaorui = new General(this, "yjcaorui$", "wei", 3);
    yjcaorui->addSkill(new YjMingmin);
    yjcaorui->addSkill(new YjPianquan);
    yjcaorui->addSkill(new YjRenxin);

    General *yjmaliang = new General(this, "yjmaliang", "shu", 3);
    yjmaliang->addSkill(new YjNaman);
    yjmaliang->addSkill(new YjZhiman);
    yjmaliang->addSkill(new YjYinjian);
    addMetaObject<YjYinjianCard>();

    skills << new YjYoujin << new YjYoujinPlay;

    related_skills.insertMulti("yjyoujin", "#yjyoujin");
}

ADD_PACKAGE(YJ3rd)
