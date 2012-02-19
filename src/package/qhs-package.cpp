#include "qhs-package.h"
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

class YinshihBuff: public TriggerSkill{
public:
    YinshihBuff():TriggerSkill("#yinshih_buff"){
        events << CardEffect << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.to == effect.from)
            return false;

        if(effect.card->inherits("BasicCard")){

            if(not (effect.to->inMyAttackRange(effect.from)) && (effect.from->hasSkill(objectName()))){
                LogMessage log;
                log.type = "#yinshih";
                log.from = effect.from;
                log.arg = effect.card->objectName();
                room->sendLog(log);

                room->playSkillEffect(objectName());
                effect.from->drawCards(1);
                return true;
            }

            if((effect.to->hasSkill(objectName()) && not (effect.to->inMyAttackRange(effect.from)))){
                LogMessage log;
                log.type = "#yinshih";
                log.from = effect.to;
                log.arg = effect.card->objectName();
                room->sendLog(log);

                room->playSkillEffect(objectName());
                effect.to->drawCards(1);
                return true;
            }
        }
        return false;

    }
};

class Yinshih: public ProhibitSkill{
public:
    Yinshih():ProhibitSkill("yinshih"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if (to->hasSkill("yinshih"))
            return (card->inherits("DelayedTrick"));
        else{
            return false;
        }
    }
};

class Shuijingh: public TriggerSkill{
public:
    Shuijingh():TriggerSkill("shuijingh"){
        events << FinishJudge << HpLost << HpRecover << Predamaged;
        frequency = NotFrequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->findPlayerBySkillName("shuijingh");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        ServerPlayer *simahuih = room->findPlayerBySkillName(objectName());
        if(!simahuih)
            return false;

        if(event != FinishJudge){

            LogMessage log;
            log.type = "#shuijingh";
            log.arg = player->getGeneralName();
            room->sendLog(log);

            room->setTag("ShuijinghTarget", QVariant::fromValue(player));


            if(simahuih->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(.*):(.*)");
                judge.reason = objectName();
                judge.who = simahuih;

                room->judge(judge);

                const Card *card = room->askForCard(simahuih, QString(".%1").arg(judge.card->getSuitString().at(0).toUpper()), "@shuijingh1");
                room->playSkillEffect(objectName());
                if(card){
                    if(event == HpRecover){
                        RecoverStruct recover = data.value<RecoverStruct>();
                        recover.recover = recover.recover+1;
                        data = QVariant::fromValue(recover);
                        room->setEmotion(player, "good");

                        LogMessage log;
                        log.type = "$shuijingh2";
                        log.from = simahuih;
                        log.card_str = card->toString();
                        room->sendLog(log);

                        return false;
                    }
                    if(event == HpLost){
                        int lost = data.toInt();
                        lost = lost + 1;
                        data = QVariant::fromValue(lost);

                        LogMessage log;
                        log.type = "$shuijingh1";
                        log.from = simahuih;
                        log.card_str = card->toString();
                        room->sendLog(log);

                        return false;
                    }
                    if(event == Predamaged){
                        DamageStruct damage = data.value<DamageStruct>();
                        damage.damage = damage.damage+1;
                        data = QVariant::fromValue(damage);

                        LogMessage log;
                        log.type = "$shuijingh3";
                        log.from = simahuih;
                        log.card_str = card->toString();
                        room->sendLog(log);

                        return false;
                    }
                }
                else{

                    LogMessage log;
                    log.type = "$shuijingh4";
                    log.from = simahuih;
                    room->sendLog(log);

                    return false;
                }
            }
        }
        return false;

    }
};

FenbeihCard::FenbeihCard(){

}

bool FenbeihCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void FenbeihCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.to->drawCards(2);

    int n = (effect.to->getHandcardNum() +1 ) /2 ;
    int i;
    for(i=0; i<n; i++){
        int card_id = room->askForCardChosen(effect.from, effect.to, "h", "fenbeih");
        const Card *card = Sanguosha->getCard(card_id);
        room->moveCardTo(card, effect.from, Player::Hand, false);
    }
    room->setEmotion(effect.to, "bad");

    LogMessage log;
    log.type = "#fenbeih";
    log.from = effect.from;
    room->sendLog(log);


}

class FenbeihViewAsSkill: public ZeroCardViewAsSkill{
public:
    FenbeihViewAsSkill():ZeroCardViewAsSkill("fenbeih"){
    }

    virtual const Card *viewAs() const{
        return new FenbeihCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@fenbeih";
    }
};

class Fenbeih:public PhaseChangeSkill{
public:
    Fenbeih():PhaseChangeSkill("fenbeih"){
        view_as_skill = new FenbeihViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *zuocih) const{
        if(zuocih->getPhase() == Player::Draw){
            Room *room = zuocih->getRoom();

            if(room->askForUseCard(zuocih, "@@fenbeih", "@fenbeih_to")){

                room->playSkillEffect("fenbeih", 1);
                return true;
            }else
                return false;
        }
        return false;
    }
};

class Yindunh: public ProhibitSkill{
public:
    Yindunh():ProhibitSkill("yindunh"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if(!to->getPile("yindunh_pile").isEmpty()){
            QString suit_str = Sanguosha->getCard(to->getPile("yindunh_pile").first())->getSuitString();
            return (card->getSuitString() == suit_str && to->getMark("yindunhbuff") == 1);
        }
        else
            return false;

    }
};

class YindunhBuff: public TriggerSkill{
public:
    YindunhBuff():TriggerSkill("#yindunh_buff"){
        events << CardDiscarded << PhaseChange;
        frequency = NotFrequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        ServerPlayer *zuocih = room->findPlayerBySkillName("yindunh");
        if(!zuocih)
            return false;

        if(event == CardDiscarded && zuocih->getPhase() == Player::Discard){
            CardStar card = data.value<CardStar>();

            if(!card->subcardsLength()==0 && room->askForSkillInvoke(zuocih, "yindunh")){
                room->playSkillEffect("yindunh");

                room->setPlayerMark(zuocih, "yindunhbuff", 1);

                QList<int> discards;
                foreach(int card_id, card->getSubcards()){
                    if(room->getCardPlace(card_id)==Player::DiscardedPile)
                        discards << card_id;
                }

                room->fillAG(discards, zuocih);
                int ydid = room->askForAG(zuocih, discards, true, "yindunh");
                zuocih->addToPile("yindunh_pile", ydid, true);
                zuocih->invoke("clearAG");

                LogMessage log;
                log.type = "#yindunh";
                log.from = zuocih;
                log.arg = Sanguosha->getCard(ydid)->getSuitString();
                room->sendLog(log);

                return false;

            }
        }
        else
            if(event == PhaseChange && zuocih->getPhase() == Player::Start){
                room->setPlayerMark(zuocih, "yindunhbuff", 0);
                zuocih->clearPrivatePiles();
                return false;
            }
        return false;
    }
};

class Fangshuhbuff: public FilterSkill{
public:
    Fangshuhbuff():FilterSkill("fangshuh_buff"){
        frequency = Compulsory;

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        if(Self->getPile("fangshuh_pile").isEmpty()){
            return NULL;
        }else{
            const Card *fs_card = Sanguosha->getCard(Self->getPile("fangshuh_pile").first());
            Card *new_card = Sanguosha->cloneCard(fs_card->objectName(), fs_card->getSuit(), fs_card->getNumber());
            new_card->setSkillName(objectName());
            new_card->addSubcard(card_item->getCard());

            return new_card;
        }
    }
};

FangshuhCard::FangshuhCard(){
    once = true;
}

bool FangshuhCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void FangshuhCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    int card_id = this->getEffectiveId();
    target->addToPile("fangshuh_pile", card_id, true);
    room->attachSkillToPlayer(target, "fangshuh_buff");
}

class FangshuhViwAsSkill: public ViewAsSkill{
public:
    FangshuhViwAsSkill():ViewAsSkill("fangshuh"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("FangshuhCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return ((to_select->getFilteredCard()->inherits("BasicCard")) &&
                ((to_select->getFilteredCard()->getSuit() != Card::Heart)));
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        FangshuhCard *card = new FangshuhCard;
        card->addSubcards(cards);
        return card;
    }
};

class Fangshuh: public TriggerSkill{
public:
    Fangshuh():TriggerSkill("fangshuh"){
        events << PhaseChange << CardLost;
        view_as_skill = new FangshuhViwAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == PhaseChange && player->getPhase() == Player::Finish && player->hasSkill("fangshuh_buff")){
            room->detachSkillFromPlayer(player, "fangshuh_buff");
            player->clearPrivatePiles();
            return false;
        }else if(event == CardLost && player->hasSkill("fangshuh_buff")){
            CardMoveStar move = data.value<CardMoveStar>();

            QList<int>  fangshuh = player->getPile("fangshuh_pile");

            if(fangshuh.isEmpty())
                return false;

            if((Sanguosha->getCard(fangshuh.first())->inherits("Shit"))
                    && (move->from_place == Player::Hand)
                    && (move->to_place == Player::DiscardedPile)){

                const Card *card = Sanguosha->getCard(move->card_id);

                DamageStruct damage;
                damage.card = card;
                damage.from = move->from;
                damage.to = move->from;

                if(Sanguosha->getCard(fangshuh.first())->getSuit() == Card::Club){
                    damage.nature = DamageStruct::Thunder;
                }else if(Sanguosha->getCard(fangshuh.first())->getSuit() == Card::Heart){
                    damage.nature = DamageStruct::Fire;
                }else if((Sanguosha->getCard(fangshuh.first())->getSuit() == Card::Diamond)
                         || (Sanguosha->getCard(fangshuh.first())->getSuit() == Card::Spade)){
                    damage.nature = DamageStruct::Normal;
                }

                room->damage(damage);
                return false;
            }
            return false;
        }else
            return false;
        return false;
    }
};

class Xiurenh: public TriggerSkill{
public:
    Xiurenh():TriggerSkill("xiurenh"){
        events << Predamage;
        frequency = NotFrequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(player->hasFlag("xiurenused"))
            return false;
        DamageStruct damage = data.value<DamageStruct>();

        if(!room->askForSkillInvoke(player, objectName()))
            return false;

        room->playSkillEffect(objectName());
        int i;
        for(i=0; i<2; i++){
            if(damage.to->isKongcheng()){
                QString choice2 = room->askForChoice(player, objectName(), "rec1hp+dra2cd");

                if(choice2 == "rec1hp"){
                    RecoverStruct recover;
                    recover.card = NULL;
                    recover.who = player;
                    recover.recover = 1;
                    room->recover(player, recover, true);
                }else
                    player->drawCards(2);
                break;
            }else{
                QString choice = room->askForChoice(damage.to, objectName(), "show+giveup");

                if(choice == "show"){
                    const Card *card = room->askForCardShow(damage.to, player, objectName());
                    ServerPlayer *to = room->askForPlayerChosen(player, room->getOtherPlayers(damage.to), "xiurenh");
                    room->moveCardTo(card, to, Player::Hand, true);
                }else{
                    QString choice2 = room->askForChoice(player, objectName(), "rec1hp+dra2cd");

                    if(choice2 == "rec1hp"){
                        RecoverStruct recover;
                        recover.card = NULL;
                        recover.who = player;
                        recover.recover = 1;
                        room->recover(player, recover, true);
                    }else
                        player->drawCards(2);
                    break;
                }
            }
        }
        player->setFlags("xiurenused");
        return true;
    }
};

class Shushenh: public TriggerSkill{
public:
    Shushenh():TriggerSkill("shushenh"){
        events << Predamaged;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();

        int delta = qMin(damage.damage, qAbs(player->getHandcardNum()-player->getHp()));
        damage.damage = delta;
        data = QVariant::fromValue(damage);
        room->playSkillEffect(objectName());

        LogMessage log;
        log.type = "#shushenh";
        log.from = player;
        log.arg = QString::number(delta);
        room->sendLog(log);

        return false;
    }
};

class Keshih: public TriggerSkill{
public:
    Keshih():TriggerSkill("keshih"){
        events << SlashMissed;
        frequency = NotFrequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return ! target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        ServerPlayer *wangyih = room->findPlayerBySkillName(objectName());
        if(!wangyih)
            return false;

        if((effect.from) && (effect.to == wangyih) && (room->askForSkillInvoke(wangyih, objectName()))){
            room->playSkillEffect(objectName());

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(.*):(.*)");
            judge.reason = objectName();
            judge.who = wangyih;
            room->judge(judge);

            if((judge.card->isBlack()) && (!judge.card->inherits("TrickCard"))){
                room->moveCardTo(judge.card, effect.from, Player::Judging, true);
            }else if(judge.card->getSuit() == Card::Diamond){
                wangyih->obtainCard(judge.card);
            }
            return false;
        }
        return false;
    }
};

class Mingjih: public TriggerSkill{
public:
    Mingjih():TriggerSkill("mingjih"){
        events << CardUsed << PhaseChange;
        frequency = NotFrequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *wangyih = room->findPlayerBySkillName(objectName());
        if(!wangyih)
            return false;

        if((event == CardUsed) && (wangyih->getPhase()==Player::NotActive)){
            CardUseStruct use = data.value<CardUseStruct>();
            if((use.card->isNDTrick())
                    && (!wangyih->isKongcheng())
                    && (room->askForSkillInvoke(wangyih, objectName()))
                    && (room->askForDiscard(wangyih, objectName(), 1, false, false))){
                room->playSkillEffect(objectName());
                const Card *card = Sanguosha->getCard(room->drawCard());
                room->moveCardTo(card, NULL, Player::Special, true);

                LogMessage log;
                log.type = "$mingjih_show";
                log.from = wangyih;
                log.card_str = card->toString();
                room->sendLog(log);

                if(card->getNumber() <= 9){

                    LogMessage log;
                    log.type = "#mingjih_bad";
                    log.from = wangyih;
                    log.to << use.to;
                    log.arg = use.card->objectName();
                    log.arg2 = card->getNumberString();
                    room->sendLog(log);

                    wangyih->obtainCard(card);
                    wangyih->addToPile("mingjih_pile", card->getEffectiveId(), true);

                    return false;
                }else{

                    LogMessage log;
                    log.type = "#mingjih_good";
                    log.from = wangyih;
                    log.to << use.to;
                    log.arg = use.card->objectName();
                    log.arg2 = card->getNumberString();
                    room->sendLog(log);

                    room->setEmotion(use.from, "bad");
                    use.from->obtainCard(card);
                    wangyih->obtainCard(use.card);
                    wangyih->addToPile("mingjih_pile", use.card->getEffectiveId(), true);

                    return true;
                }
            }

        }else if((event == PhaseChange) && (wangyih->getPhase() == Player::Start)){
            QList<int> mingjih_pile = wangyih->getPile("mingjih_pile");
            if(mingjih_pile.isEmpty())
                return false;

            if(!room->askForSkillInvoke(wangyih, objectName()))
                return false;

            room->playSkillEffect(objectName());
            int card_id;
            if(mingjih_pile.length() == 1)
                card_id = mingjih_pile.first();
            else{
                room->fillAG(mingjih_pile, wangyih);
                card_id = room->askForAG(wangyih, mingjih_pile, true, "mingjih");
                wangyih->invoke("clearAG");

                if(card_id == -1)
                    return false;
            }
            room->moveCardTo(Sanguosha->getCard(card_id), wangyih, Player::Hand, true);
            room->setPlayerFlag(wangyih, "miingjih");
            wangyih->drawCards(1);
            wangyih->skip(Player::Draw);

            return false;
        }
        return false;
    }
};



class Midaoh: public TriggerSkill{
public:
    Midaoh():TriggerSkill("midaoh"){
        
        events << StartJudge;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();
        ServerPlayer *p = room->findPlayerBySkillName(objectName());
        if(!p)
            return false;

        p->tag["Judge"] = data;

        if(p->getPile("rice").isEmpty())
            return false;

        if(room->askForSkillInvoke(p, objectName()))
        {
            room->playSkillEffect(objectName());
            QList<int> rice = p->getPile("rice");
            int card_id;
            if(rice.length() == 1)
                card_id = rice.first();
            else{
                room->fillAG(rice, p);
                card_id = room->askForAG(p, rice, true, "midao");
                p->invoke("clearAG");
            }

            const Card *card = Sanguosha->getCard(card_id);

            if(card){

                judge->card = Sanguosha->getCard(card->getEffectiveId());
                room->moveCardTo(judge->card, NULL, Player::Special);

                LogMessage log;
                log.type = "$ChangedJudge";
                log.from = p;
                log.to << judge->who;
                log.card_str = card->getEffectIdString();
                room->sendLog(log);

                room->sendJudgeResult(judge);

                return true;
            }

            return false;
        }
        return false;
    }
};

class Mengyunh:public OneCardViewAsSkill{
public:
    Mengyunh():OneCardViewAsSkill("mengyunh"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("BasicCard");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "jink" || pattern == "slash";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        switch(ClientInstance->getStatus()){
        case Client::Playing:{
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card);
            slash->setSkillName(objectName());
            return slash;
        }
        case Client::Responsing:{
            QString pattern = ClientInstance->getPattern();
            if(pattern == "jink"){
                Jink *jink = new Jink(card->getSuit(), card->getNumber());
                jink->addSubcard(card);
                jink->setSkillName(objectName());
                return jink;
            }else if(pattern == "slash"){
                Slash *slash = new Slash(card->getSuit(), card->getNumber());
                slash->addSubcard(card);
                slash->setSkillName(objectName());
                return slash;
            }
        }
        default:
            return NULL;
        }
    }
};

class Yumah: public DistanceSkill{
public:
    Yumah():DistanceSkill("yumah"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(from->hasSkill(objectName()) && (to->getDefensiveHorse()))
            return -1;
        else if(to->hasSkill(objectName()) && (from->getOffensiveHorse()))
            return 1;
        else
            return 0;
    }
};

class Mingshenh: public TriggerSkill{
public:
    Mingshenh():TriggerSkill("mingshenh"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("mingshenh");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        ServerPlayer *xinxianyingh = room->findPlayerBySkillName(objectName());
        if(!xinxianyingh)
            return false;

        if(effect.from != xinxianyingh && effect.card->isNDTrick()){
            room->playSkillEffect(objectName());
            if(effect.to->isKongcheng())
                return true;
            else{
                const Card *card = Sanguosha->cloneCard("fire_attack", effect.card->getSuit(), effect.card->getNumber());

                LogMessage log;
                log.type = "#mingshenh";
                log.from = xinxianyingh;
                log.arg = effect.card->objectName();
                log.arg2 = card->objectName();
                room->sendLog(log);

                effect.card = card;
                data = QVariant::fromValue(effect);
                return false;
            }
        }
        return false;
    }
};

CaijianhCard::CaijianhCard(){

}

bool CaijianhCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty())
        return ((!to_select->getJudgingArea().length() == 0)
                || (!to_select->getEquips().length() == 0));
    else
        return false;
}

void CaijianhCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->throwCard(this);

    int card_id = room->askForCardChosen(effect.from, effect.to, "ej", "caijianh");
    const Card *card = Sanguosha->getCard(card_id);
    Indulgence *indulgence = new Indulgence(Card::NoSuit, 0);
    SupplyShortage *supplyshortage = new SupplyShortage(Card::NoSuit, 0);

    bool canmove = true;
    if(card->getSuit() == Card::Diamond){
        if(effect.from->isProhibited(effect.to, indulgence) && effect.to->containsTrick("indulgence"))
            canmove = false;
    }else if(card->isBlack()){
        if(effect.from->isProhibited(effect.to, supplyshortage) && effect.to->containsTrick("supply_shortage"))
            canmove = false;
    }else if(card->getSuit() == Card::Heart)
        canmove = false;

    if(canmove == true)
        room->moveCardTo(card, effect.to, room->getCardPlace(card_id) == Player::Equip ? Player::Judging : Player::Hand, true);
    else
        room->moveCardTo(card, effect.from, Player::Hand, true);
}

class Caijianh:public OneCardViewAsSkill{
public:
    Caijianh():OneCardViewAsSkill("caijianh"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->isRed();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("CaijianhCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new CaijianhCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

ZhixinhCard::ZhixinhCard(){
    target_fixed = true;
}

void ZhixinhCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *to = room->askForPlayerChosen(source, room->getOtherPlayers(source), "zhixinh");
    QString role = to->getRole();
    room->askForChoice(source, "zhixinhresult", QString("%1+Ok").arg(role));
    source->loseMark("@zhixinh");
    room->throwCard(this);
}

class Zhixinh: public ZeroCardViewAsSkill{
public:
    Zhixinh():ZeroCardViewAsSkill("zhixinh"){
    }

    virtual const Card *viewAs() const{
        return new ZhixinhCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@zhixinh") == 1;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  false;
    }
};

FenlihCard::FenlihCard(){
    once = true;
}

bool FenlihCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select->isNude())
        return false;

    else
        return true;
}

bool FenlihCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return !targets.isEmpty();
}

void FenlihCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    ServerPlayer *kongrongh = source;

    QList<int> cards ;
    kongrongh->drawCards(1);
    cards << kongrongh->handCards().mid(kongrongh->getHandcardNum() - 1);


    foreach(ServerPlayer *p, targets){
        room->fillAG(cards, kongrongh);
        int card_id = room->askForCardChosen(kongrongh, p, "hej", "fenlih");
        cards << card_id;
        kongrongh->addToPile("fenlihPile", card_id, room->getCardPlace(card_id) == Player::Hand ? false: true);
        //room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, false);
        kongrongh->invoke("clearAG");
    }

    QList<ServerPlayer *> fenlihto;
    fenlihto << kongrongh << targets;
    foreach(ServerPlayer *p, fenlihto){
        room->fillAG(cards, kongrongh);
        int cdid = room->askForAG(kongrongh, cards, true, "fenlih");
        room->moveCardTo(Sanguosha->getCard(cdid), p, Player::Hand, false);
        kongrongh->invoke("clearAG");
        cards.removeOne(cdid);
    }
}

class Fenlih: public ZeroCardViewAsSkill{
public:
    Fenlih():ZeroCardViewAsSkill("fenlih"){
    }

    virtual const Card *viewAs() const{
        return new FenlihCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("FenlihCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  false;
    }
};

class Zhongyongh: public TriggerSkill{
public:
    Zhongyongh():TriggerSkill("zhongyongh"){
        events << Damage << Damaged;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *chendaoh = room->findPlayerBySkillName(objectName());
        if(!chendaoh)
            return false;
        room->playSkillEffect(objectName());
        DamageStruct damage = data.value<DamageStruct>();
        if(chendaoh && chendaoh->isAlive()){
            int i, x = damage.damage;
            for(i=0; i<x; i++){
                if(chendaoh->getPile("zhongyongh_pile").length() == 5)
                    break;
                int cdid = room->drawCard();
                chendaoh->addToPile("zhongyongh_pile", cdid, true);
            }
        }else
            return false;
        return false;
    }
};

class ZhongyonghGet: public TriggerSkill{
public:
    ZhongyonghGet():TriggerSkill("#zhongyonghget"){
        events << PhaseChange;
        frequency = NotFrequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("zhongyongh");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *chendaoh = room->findPlayerBySkillName("zhongyongh");
        if(!chendaoh)
            return false;
        QList<int> zhongyongh_pile = chendaoh->getPile("zhongyongh_pile");

        if(event == PhaseChange && chendaoh->getPhase() == Player::Start){
            if(zhongyongh_pile.isEmpty()){
                return false;

            }else{
                if(room->askForSkillInvoke(chendaoh, "zhongyongh", data)){
                    if(zhongyongh_pile.length() == 1)
                        room->moveCardTo(Sanguosha->getCard(zhongyongh_pile.first()), chendaoh, Player::Hand, false);
                    else{
                        int i;
                        for(i=0; i<qMax(2, chendaoh->getLostHp()); i++){
                            room->fillAG(zhongyongh_pile, chendaoh);
                            int cdid = room->askForAG(chendaoh, zhongyongh_pile, false, "zhongyongh");
                            room->moveCardTo(Sanguosha->getCard(cdid), chendaoh, Player::Hand, false);
                            chendaoh->invoke("clearAG");
                            zhongyongh_pile.removeOne(cdid);
                        }
                    }
                    chendaoh->skip(Player::Draw);
                    return false;
                }else
                    return false;
            }
        }
        return false;
    }
};

class Huweih: public TriggerSkill{
public:
    Huweih():TriggerSkill("huweih"){
        events << SlashEffected;
        frequency = NotFrequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *chendaoh = room->findPlayerBySkillName(objectName());
        if(!chendaoh)
            return false;
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(chendaoh->isKongcheng())
            return false;

        if(!room->askForSkillInvoke(chendaoh, objectName(), data)) return false;
        room->playSkillEffect(objectName());
        int id1 = room->askForCardShow(chendaoh, chendaoh, "huweih")->getId();
        chendaoh->addToPile("huweih_pile", id1, false);
        int id2, x1, x2, x3 ;
        QString choice1 = room->askForChoice(effect.from, "pleaseselect", "stickslash+cancleslash");

        LogMessage log;
        log.type = "#huweih1";
        log.from = effect.from;
        log.arg = choice1;
        room->sendLog(log);

        if(choice1 == "cancleslash"){
            room->throwCard(id1);
            return true;
        }else{
            if(effect.from->isKongcheng()){
                x1 = Sanguosha->getCard(id1)->getNumber();
                x2 = 0;
                x3 = effect.slash->getNumber();
            }else{
                QString choice2 = room->askForChoice(effect.from, "pleaseselect", "huweih_yes+huweih_no");

                LogMessage log;
                log.type = "#huweih2";
                log.from = effect.from;
                log.arg = choice2;
                room->sendLog(log);

                if(choice2 == "huweih_yes"){
                    id2 = room->askForCardShow(effect.from, effect.from, "huweih")->getId();
                    effect.from->addToPile("huweih_pile", id2, false);
                    x1 = Sanguosha->getCard(id1)->getNumber();
                    x2 = Sanguosha->getCard(id2)->getNumber();
                    x3 = effect.slash->getNumber();
                    room->throwCard(id2);
                }else{
                    x1 = Sanguosha->getCard(id1)->getNumber();
                    x2 = 0;
                    x3 = effect.slash->getNumber();
                }
            }
            room->throwCard(id1);

            LogMessage log;
            log.type = "#huweih3";
            log.from = chendaoh;
            log.to << effect.from;
            log.arg = QString::number(x1);
            log.arg2 = QString::number(x2+x3);
            room->sendLog(log);

            if(x1 >= x2+x3){

                LogMessage log;
                log.type = "#huweih4";
                log.from = chendaoh;
                log.to << effect.from;
                room->sendLog(log);

                DamageStruct damage;
                damage.from = chendaoh;
                damage.to = effect.from;
                damage.damage = 1;
                room->damage(damage);
                return true;
            }else if((x1 < x2+x3) && (x1 > (x2+x3)/2)){
                LogMessage log;
                log.type = "#huweih5";
                log.from = chendaoh;
                log.to << effect.from;
                room->sendLog(log);

                return false;
            }else if((x1 < x2+x3) && (x1 < (x2+x3)/2)){
                LogMessage log;
                log.type = "#huweih6";
                log.from = chendaoh;
                log.to << effect.from;
                room->sendLog(log);

                room->slashResult(effect, NULL);
                return true;
            }
            return false;
        }
    }
};

ZhenluanhCard::ZhenluanhCard(){
    target_fixed = true;
}

void ZhenluanhCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
}

class ZhenluanhViewAsSkill:public OneCardViewAsSkill{
public:
    ZhenluanhViewAsSkill():OneCardViewAsSkill("zhenluanh"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@zhenluanh";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new ZhenluanhCard;
        card->setSuit(card_item->getFilteredCard()->getSuit());
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Zhenluanh: public TriggerSkill{
public:
    Zhenluanh():TriggerSkill("zhenluanh"){
        view_as_skill = new ZhenluanhViewAsSkill;
        events << SlashProceed;
        frequency = NotFrequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *huangfusongh = room->findPlayerBySkillName(objectName());
        if(!huangfusongh)
            return false;
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        room->playSkillEffect(objectName());

        const Card *card = room->askForCard(effect.to, "slash", "@zhenluanh-slash", data);

        if(card){
            if(room->askForCard(huangfusongh, "@zhenluanh", "@zhenluanh-card", data)){
                Duel *duel = new Duel(Card::NoSuit, 0);
                duel->setSkillName(objectName());
                room->cardEffect(duel, huangfusongh, effect.to);
            }
            return true;
        }else{
            DamageStruct damage;
            damage.from = huangfusongh;
            damage.card = effect.slash;
            damage.to = effect.to;
            damage.nature = effect.nature;
            if (effect.drank)
                damage.damage = 2;
            else
                damage.damage = 1;
            room->damage(damage);
            return true;
        }
        return false;
    }
};

GuijihCard::GuijihCard(){
    once = true;
}

bool GuijihCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return true;
}

bool GuijihCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void GuijihCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target1 = targets.at(0);
    ServerPlayer *target2 = targets.at(1);
    target1->obtainCard(this);

    const Card *slash1 = NULL;
    slash1 = room->askForCard(target1, "slash", "@guijih-slash");
    if(slash1 == NULL){
        DamageStruct damage;
        damage.from = source;
        damage.to = target1;
        damage.damage = 1;
        room->damage(damage);
    }else{
        CardUseStruct use;
        use.from = target1;
        use.to << target2;
        use.card = slash1;
        room->useCard(use);

        const Card *slash2 = NULL;
        slash2 = room->askForCard(target2, "slash", "@guijih-slash");
        if(slash2 != NULL){
            CardUseStruct use;
            use.from = target2;
            use.to << target1;
            use.card = slash2;
            room->useCard(use);
        }
    }
}

class Guijih: public OneCardViewAsSkill{
public:
    Guijih():OneCardViewAsSkill("guijih"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("GuijihCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        GuijihCard *acard = new GuijihCard;
        acard->addSubcard(card_item->getCard()->getId());
        acard->setSkillName(objectName());

        return acard;
    }
};

class Ziaoh: public TriggerSkill{
public:
    Ziaoh():TriggerSkill("ziaoh"){
        events << SlashHit;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        ServerPlayer *wangyunh = room->findPlayerBySkillName(objectName());
        if(!wangyunh)
            return false;

        if(effect.to != wangyunh)
            return false;

        LogMessage log;
        log.type = "#ziaoh";
        log.from = player;
        log.to << effect.from;
        room->sendLog(log);

        if(room->askForDiscard(effect.from, "@ziaoh", 1, true, false))
            return false;
        else
            return true;
    }
};

class Qianzhih: public TriggerSkill{
public:
    Qianzhih():TriggerSkill("qianzhih"){
        events << CardResponsed;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *guohuai = room->findPlayerBySkillName(objectName());
        if(!guohuai)
            return false;
        if(event == CardResponsed && guohuai->getPhase() == Player::NotActive){
            CardStar card_star = data.value<CardStar>();
            if(!card_star->inherits("Jink"))
                return false;
            else{
                if(!room->askForSkillInvoke(guohuai, objectName(), data))
                    return false;

                room->playSkillEffect(objectName());
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade|club):(.*)");
                judge.reason = objectName();
                judge.good = true;
                judge.who = guohuai;
                room->judge(judge);

                if(judge.isGood()){
                    if(room->askForUseCard(guohuai, "Slash,Peach,TrickCard,EquipCard", "@qianzhi"))
                        return false;
                    else
                        guohuai->drawCards(1);
                    return false;
                }else
                    return false;
            }
        }
        return false;
    }
};

class Shangshih: public TriggerSkill{
public:
    Shangshih():TriggerSkill("shangshih"){
        events << PhaseChange << FinishJudge;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhangchunhuah, QVariant &data) const{
        Room *room = zhangchunhuah->getRoom();

        if(event == PhaseChange && zhangchunhuah->getPhase() == Player::Finish){
            while(!zhangchunhuah->hasFlag("samecolor") && zhangchunhuah->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                zhangchunhuah->setFlags("shangshih_used");

                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(.*):(.*)");
                judge.reason = objectName();
                judge.who = zhangchunhuah;

                room->judge(judge);

                room->setTag("PreJudgeCard", QVariant::fromValue(judge.card));
            }
            zhangchunhuah->setFlags("-shangshih_used");

        }else if(event == FinishJudge && zhangchunhuah->hasFlag("shangshih_used")){
            JudgeStar judge = data.value<JudgeStar>();
            QString color, newcolor;
            if(zhangchunhuah->getMark("shangshih_usedtimes") <= 0){
                color = "no-color";
            }else{
                const Card *prejudgecard = room->getTag("PreJudgeCard").value<CardStar>();
                room->removeTag("PreJudgeCard");
                if(prejudgecard->isRed())
                    color = "red";
                else
                    color = "black";
            }

            if(judge->card->isRed())
                newcolor = "red";
            else
                newcolor = "black";

            if(color == newcolor){
                zhangchunhuah->setFlags("samecolor");
                zhangchunhuah->setMark("shangshih_usedtimes", 0);
                return false;
            }else{
                zhangchunhuah->obtainCard(judge->card);
                zhangchunhuah->addMark("shangshih_usedtimes");
                if(zhangchunhuah->getMark("shangshih_usedtimes") == 4){
                    zhangchunhuah->turnOver();
                }
                return true;
            }
        }
        return false;
    }
};

class Jueqingh: public TriggerSkill{
public:
    Jueqingh():TriggerSkill("jueqingh"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhangchunhuah, QVariant &data) const{
        Room *room = zhangchunhuah->getRoom();
        room->playSkillEffect(objectName());

        LogMessage log;
        log.type = "#jueqingh";
        log.from = zhangchunhuah;
        room->sendLog(log);

        DamageStruct damage = data.value<DamageStruct>();
        damage.from = NULL;
        data = QVariant::fromValue(damage);

        return false;
    }
};

class Xiaozhanh: public TriggerSkill{
public:
    Xiaozhanh():TriggerSkill("xiaozhanh"){
        events << CardResponsed;
        frequency = NotFrequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *caozhangh = room->findPlayerBySkillName(objectName());
        if(!caozhangh)
            return false;
        CardStar card_star = data.value<CardStar>();

        if(card_star->inherits("Jink")){
            QList<ServerPlayer *> tos ;
            foreach(ServerPlayer *p, room->getOtherPlayers(caozhangh)){
                if(caozhangh->inMyAttackRange(p))
                    tos << p;
            }
            if(caozhangh->getHp()>2){
                if(room->askForDiscard(caozhangh, "xiaozhanh", 1, true, true)){
                    room->playSkillEffect(objectName());
                    Slash *slash = new Slash(Card::NoSuit, 0);
                    ServerPlayer *to = room->askForPlayerChosen(caozhangh, tos, "@xiaozhanh");
                    room->cardEffect(slash, caozhangh, to);
                }else
                    return false;
            }else{
                room->playSkillEffect(objectName());
                ServerPlayer *to = room->askForPlayerChosen(caozhangh, tos, "@xiaozhanh");
                DamageStruct damage;
                damage.from = caozhangh;
                damage.to = to;
                damage.damage = 1;
                room->damage(damage);
                return false;
            }
        }else
            return false;
        return false;
    }
};

class Huqiangh:public TriggerSkill{
public:
    Huqiangh():TriggerSkill("huqiangh"){
        events << SlashMissed << SlashProceed;

    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhangxiuh, QVariant &data) const{
        Room *room = zhangxiuh->getRoom();
        ServerPlayer *junlinghtarget = zhangxiuh;
        if((zhangxiuh->hasUsed("JunlinghCard")) && !zhangxiuh->hasFlag("junlingh_used")){
            junlinghtarget = room->getTag("JunlinghTarget").value<PlayerStar>();
            zhangxiuh->setFlags("junlingh_used");
        }
        if(event == SlashProceed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.from->getAttackRange() >= effect.from->getHp()
                    || junlinghtarget->getAttackRange() >= effect.from->getHp()){
                if(!effect.from->askForSkillInvoke("huqiangh", QVariant::fromValue(effect)))
                    return false;
                room->playSkillEffect(objectName());
                room->slashResult(effect, NULL);
                return true;
            }else
                return false;

        }else if(event == SlashMissed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.from->getAttackRange() < effect.from->getHp()){
                if(!effect.from->askForSkillInvoke("huqiangh", QVariant::fromValue(effect)))
                    return false;

                QList<ServerPlayer *> tos;
                foreach(ServerPlayer *p, room->getOtherPlayers(effect.to)){
                    if(effect.to->inMyAttackRange(p))
                        tos << p;
                }
                ServerPlayer *to = room->askForPlayerChosen(effect.from, tos, objectName());
                const Card *slash = effect.slash;
                room->cardEffect(slash, effect.to, to);
                return false;
            }else
                return false;
            return false;
        }
        return false;
    }
};

JunlinghCard::JunlinghCard(){
    once = true;
}

bool JunlinghCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty())
        return to_select->getKingdom() == "qun";
    else
        return false;
}

void JunlinghCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int x = effect.to->getAttackRange();
    room->setTag("JunlinghTarget", QVariant::fromValue(effect.to));
    QList<ServerPlayer *> tos;
    foreach(ServerPlayer *p, room->getOtherPlayers(effect.from)){
        if(effect.from->distanceTo(p) <= x)
            tos << p;
    }
    const Card *slash = room->askForCard(effect.from, "slash", "@junlingh_slash:");// + to->objectName());

    if(slash){
        ServerPlayer *to = room->askForPlayerChosen(effect.from, tos, "junlingh_to");
        CardUseStruct use;
        use.card = slash;
        use.to << to;
        use.from = effect.from;
        room->useCard(use, true);
    }
}

class Junlingh: public ZeroCardViewAsSkill{
public:
    Junlingh():ZeroCardViewAsSkill("junlingh$"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (!player->hasUsed("JunlinghCard"));
    }

    virtual const Card *viewAs() const{
        return new JunlinghCard;
    }
};

class Jiangjih: public TriggerSkill{
public:
    Jiangjih():TriggerSkill("jiangjih"){

        events << CardFinished;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{

        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *huangshuh = room->findPlayerBySkillName(objectName());
        if(!huangshuh)
            return false;

        CardUseStruct use = data.value<CardUseStruct>();
        if(use.to.length() == 1
                && use.to.first() == huangshuh
                && (use.card->inherits("BasicCard") || use.card->isNDTrick())
                && room->getCardPlace(use.card->getEffectiveId()) == Player::DiscardedPile){
            if(!huangshuh->askForSkillInvoke(objectName()))
                return false;
            room->moveCardTo(use.card, huangshuh, Player::Hand, true);
            return false;
        }else
            return false;
        return false;
    }
};

class Jiujih: public TriggerSkill{
public:
    Jiujih():TriggerSkill("jiujih"){
        events << CardGot << CardGotDone;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{

        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *huangshuh, QVariant &data) const{
        Room *room = huangshuh->getRoom();
        if(event == CardGot && huangshuh->getPhase() == Player::NotActive){
            CardMoveStar move = data.value<CardMoveStar>();
            const Card *card = Sanguosha->getCard(move->card_id);
            if(move->to_place == Player::Judging || move->card_id == -1)
                return false;
            if(card->inherits("Slash") || card->inherits("Duel")){
                huangshuh->tag["InvokeJiuji"] = true;
                room->setTag("JiuJiCard", QVariant::fromValue(card));
                return false;
            }else
                return false;
        }else if(event == CardGotDone){
            if(!huangshuh->tag.value("InvokeJiuji", false).toBool())
                return false;
            huangshuh->tag.remove("InvokeJiuji");
            QList<ServerPlayer *> tos;
            foreach(ServerPlayer *p, room->getOtherPlayers(huangshuh)){
                if(!p->isKongcheng())
                    tos << p;
            }
            if(tos.isEmpty())
                return false;
            if(!huangshuh->askForSkillInvoke(objectName(), data))
                return false;
            const Card *card = room->getTag("JiuJiCard").value<CardStar>();
            int card_id = card->getEffectiveId();
            room->showCard(huangshuh, card_id);
            ServerPlayer *to = room->askForPlayerChosen(huangshuh, tos, objectName());
            int to_card_id = room->askForCardChosen(huangshuh, to, "h", objectName());
            room->showCard(to, to_card_id);
            if(card->sameColorWith(Sanguosha->getCard(to_card_id))){
                CardUseStruct use;
                use.from = huangshuh;
                use.to << to;
                use.card = card;
                room->useCard(use, false);
            }
            room->removeTag("JiuJiCard");
            return false;
        }
        return false;
    }
};

class Shanshouh: public TriggerSkill{
public:
    Shanshouh():TriggerSkill("shanshouh"){
        events << CardEffected << Damaged;
        frequency = NotFrequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == CardEffected && player->faceUp()){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->isBlack()
                    && (effect.card->isNDTrick() || effect.card->inherits("Slash"))
                    && effect.from != effect.to){
                if(!player->askForSkillInvoke(objectName(), data))
                    return false;
                player->turnOver();
                return true;
            }else
                return false;
        }else if(event == Damaged && !player->faceUp()){
            if(!player->askForSkillInvoke(objectName(), data))
                return false;
            player->turnOver();
            return false;
        }
        return false;
    }
};

class Judih: public TriggerSkill{
public:
    Judih():TriggerSkill("judih"){
        events << TurnedOver;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(!player->askForSkillInvoke(objectName(), data))
            return false;
        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(club|spade):(.*)");
        judge.good = true;
        judge.reason = objectName();
        judge.who = player;
        room->judge(judge);

        if(judge.isGood()){
            player->drawCards(1);
        }
        return false;
    }
};

class Caiaoh: public TriggerSkill{
public:
    Caiaoh():TriggerSkill("caiaoh"){
        events << PhaseChange << Predamaged << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == PhaseChange && player->getPhase() == Player::Finish){
            if(player->askForSkillInvoke(objectName(), data)){
                player->turnOver();
            }
            return false;
        }else if(event == Predamaged){
            if(!player->faceUp()){
                DamageStruct damage = data.value<DamageStruct>();
                damage.damage = 1;
                data = QVariant::fromValue(damage);
                return false;
            }
            return false;
        }else if(event == Damaged){
            if(player->isAlive() && !player->faceUp()){
                player->drawCards(player->getLostHp() + 2);
                return false;
            }
            return false;
        }
        return false;
    }
};

class Gushangh: public TriggerSkill{
public:
    Gushangh():TriggerSkill("gushangh"){
        events << CardLostDone << CardGotDone << CardDrawnDone;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if((event == CardGotDone || event == CardDrawnDone) && player->getPhase() == Player::NotActive){
            if(player->getHandcardNum() > 5){
                int x = player->getHandcardNum()/2;
                QList<int> hcids;
                foreach(const Card *card, player->getHandcards()){
                    int cdid = card->getEffectiveId();
                    hcids << cdid;
                }
                int i;
                for(i=0; i<x; i++){
                    ServerPlayer *to = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
                    room->fillAG(hcids, player);
                    int id = room->askForAG(player, hcids, false, objectName());
                    room->moveCardTo(Sanguosha->getCard(id), to, Player::Hand, false);
                    hcids.removeOne(id);
                    player->invoke("clearAG");
                }
                return false;
            }
            return false;
        }else if(event == CardLostDone){
            if(player->isKongcheng()){
                RecoverStruct recover;
                recover.card = NULL;
                recover.who = player;
                recover.recover = 1;
                room->recover(player, recover, true);
                return false;
            }
            return false;
        }
        return false;
    }
};

class Zhenzhih: public TriggerSkill{
public:
    Zhenzhih():TriggerSkill("zhenzhih"){
        events << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        LogMessage log;
        log.type = "#Zhenzhih";
        log.from = player;
        if(damage.damage < 3){
            if(!damage.from)
                return false;
            ServerPlayer *source = damage.from;
            if(damage.card && damage.card->inherits("SavageAssault")){
                foreach(ServerPlayer *p, room->getAllPlayers()){
                    if(p->hasSkill("huoshou"))
                        source = p;
                }
            }
            if(!player->isAlive())
                return false;
            if(!player->askForSkillInvoke(objectName(), data))
                return false;
            room->loseHp(source, damage.damage);
            source->drawCards(damage.damage);
            player->drawCards(damage.damage);
            log.to << source;
            room->sendLog(log);
        }else{
            if(!player->isAlive())
                return false;
            if(!player->askForSkillInvoke(objectName(), data))
                return false;
            ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
            player->drawCards(damage.damage);
            player->turnOver();
            log.to << target;
            room->sendLog(log);
            room->killPlayer(target);
            target->setAlive(false);
            room->setPlayerProperty(target, "hp", QVariant(0));
        }
        return false;
    }
};

class Lianyuh: public TriggerSkill{
public:
    Lianyuh():TriggerSkill("lianyuh"){
        events << HpRecover;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(!player->isChained())
            return false;
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if(p->isChained() && p->isWounded())
                targets << p;
        }
        if(targets.isEmpty())
            return false;
        if(!player->askForSkillInvoke(objectName(), data))
            return false;
        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
        const Card *card = room->askForCard(target, ".H", "@lianyuh");
        if(card){
            RecoverStruct recover;
            recover.who = player;
            recover.recover = 1;
            recover.card = NULL;
            room->recover(target, recover, true);
            LogMessage log;
            log.type = "$Lianyuh";
            log.from = target;
            log.card_str = card->toString();
            room->sendLog(log);
        }else
            return false;
        return false;
    }
};

class Huanhunh: public TriggerSkill{
public:
    Huanhunh():TriggerSkill("#huanhunh"){
        events << Death << TurnStart;
        frequency = Limited;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;//target->getRoom()->findPlayerBySkillName(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *skillowner = room->findPlayerBySkillName(objectName());
        if(!skillowner)
            return false;

        static QStringList deadnames;
        if(event == Death){
            deadnames << player->getGeneralName();
            return false;
        }else if(event == TurnStart && room->getCurrent() == skillowner){
            if(deadnames.isEmpty()
                    || skillowner->faceUp()
                    || skillowner->getMark("@huanhunh") != 1
                    || !skillowner->isAlive())
                return false;
            if(!skillowner->askForSkillInvoke(objectName(), data))
                return false;
            QString choice;
            if(deadnames.length() == 1)
                choice = deadnames.first();
            else
                choice = room->askForChoice(skillowner, "huanhunh", deadnames.join("+"));

            ServerPlayer *target = room->findPlayer(choice, true);
            room->revivePlayer(target);
            room->setPlayerProperty(target, "hp", QVariant(target->getMaxHP()));
            target->drawCards(target->getMaxHP());
            skillowner->loseMark("@huanhunh", 1);
            LogMessage log;
            log.type = "#Huanhunh";
            log.from = skillowner;
            log.to << target;
            room->sendLog(log);
            return true;
        }
        return false;
    }
};

QHSPackage::QHSPackage()
    :Package("QHS")
{
    General *simahuih = new General(this, "simahuih", "qun", 3);
    simahuih->addSkill(new Yinshih);
    simahuih->addSkill(new YinshihBuff);
    simahuih->addSkill(new Shuijingh);

    related_skills.insertMulti("yinshih", "#yinshih_buff");

    General *jipingh = new General(this, "jipingh", "qun", 3);
    jipingh->addSkill(new Zhenzhih);
    jipingh->addSkill(new Lianyuh);
    jipingh->addSkill(new Huanhunh);
    jipingh->addSkill(new MarkAssignSkill("@huanhunh", 1));

    related_skills.insertMulti("huanhunh", "#@huanhuanh-1");

    General *zuocih = new General(this, "zuocih", "qun", 3);
    zuocih->addSkill(new Fenbeih);
    zuocih->addSkill(new Yindunh);
    zuocih->addSkill(new YindunhBuff);
    zuocih->addSkill(new Fangshuh);

    related_skills.insertMulti("yindunh", "#yindunh_buff");

    General *ganfurenh = new General(this, "ganfurenh", "shu", 3, false);
    ganfurenh->addSkill(new Shushenh);
    ganfurenh->addSkill(new Xiurenh);

    General *wangyih = new General(this, "wangyih", "wei", 3, false);
    wangyih->addSkill(new Mingjih);
    wangyih->addSkill(new Keshih);

    General *zhanggongqih = new General(this, "zhanggongqih", "qun", 3);
    zhanggongqih->addSkill(new Midaoh);
    zhanggongqih->addSkill("xiliang");
    zhanggongqih->addSkill("yishe");

    General *mayunluh = new General(this, "mayunluh", "qun", 4, false);
    mayunluh->addSkill(new Mengyunh);
    mayunluh->addSkill(new Yumah);

    General *xinxianyingh = new General(this, "xinxianyingh", "wei", 3, false);
    xinxianyingh->addSkill(new Caijianh);
    xinxianyingh->addSkill(new Mingshenh);
    xinxianyingh->addSkill(new Zhixinh);
    xinxianyingh->addSkill(new MarkAssignSkill("@zhixinh", 1));

    related_skills.insertMulti("zhixinh", "#@zhixinh-1");

    General *kongrongh = new General(this, "kongrongh", "qun", 4);
    kongrongh->addSkill(new Fenlih);

    General *chendaoh = new General(this, "chendaoh", "shu", 4);
    chendaoh->addSkill(new Zhongyongh);
    chendaoh->addSkill(new ZhongyonghGet);
    chendaoh->addSkill(new Huweih);

    related_skills.insertMulti("zhongyongh", "#zhongyonghget");

    General *huangfusongh = new General(this, "huangfusongh", "qun", 4);
    huangfusongh->addSkill(new Zhenluanh);

    General *wangyunh = new General(this, "wangyunh", "qun", 3);
    wangyunh->addSkill(new Guijih);
    wangyunh->addSkill(new Ziaoh);

    General *guohuaih = new General(this, "guohuaih", "wei", 4);
    guohuaih->addSkill(new Qianzhih);

    General *caozhangh = new General(this, "caozhangh", "wei", 4);
    caozhangh->addSkill(new Xiaozhanh);

    General *zhangxiuh = new General(this, "zhangxiuh$", "qun", 4);
    zhangxiuh->addSkill(new Huqiangh);
    zhangxiuh->addSkill(new Junlingh);

    General *huangshuh = new General(this, "huangshuh", "qun", 3);
    huangshuh->addSkill(new Jiangjih);
    huangshuh->addSkill(new Jiujih);

    General *haozhaoh = new General(this, "haozhaoh", "wei", 4);
    haozhaoh->addSkill(new Shanshouh);
    haozhaoh->addSkill(new Judih);

    //General *liubah = new General(this, "liubah", "shu", 3);
    //liubah->addSkill(new Caiaoh);
    //liubah->addSkill(new Gushangh);

    skills << new Fangshuhbuff;

    addMetaObject<FenbeihCard>();
    addMetaObject<FangshuhCard>();
    addMetaObject<ZhixinhCard>();
    addMetaObject<FenlihCard>();
    addMetaObject<ZhenluanhCard>();
    addMetaObject<GuijihCard>();
    addMetaObject<JunlinghCard>();
    addMetaObject<CaijianhCard>();
}

ADD_PACKAGE(QHS)

