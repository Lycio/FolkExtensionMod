#include "YJ1st-package.h"
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

class YJXiJun:public SlashBuffSkill{
public:
    YJXiJun():SlashBuffSkill("xijun"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *YJmateng = effect.from;

        Room *room = YJmateng->getRoom();
        if(effect.from->askForSkillInvoke("xijun", QVariant::fromValue(effect))){
            room->playSkillEffect(objectName());

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(.*):(.*)");
            judge.reason = objectName();
            judge.who = YJmateng;
            room->judge(judge);
            const Card *jink = room->askForCard(effect.to, "jink", "@xijun-jink:"+YJmateng->objectName());
            if((jink) && (!judge.card->sameColorWith(jink))){
                room->slashResult(effect, jink);
                return true;
            } else{
                room->slashResult(effect, NULL);
                return true;
            }
        }
        return false;
    }
};

class YJYiJu:public TriggerSkill{
public:
    YJYiJu():TriggerSkill("yiju"){
        events << CardLost << CardLostDone;

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *YJmateng = room->findPlayerBySkillName(objectName());
        if(YJmateng->getPhase() == Player::NotActive)
            return false;
        if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->to_place == Player::DiscardedPile){
                const Card *card = Sanguosha->getCard(move->card_id);
                player->tag["InvokeTuntian"] = true;
                room->setTag("YiJuCard", QVariant::fromValue(card));
            }
        }else if(event == CardLostDone){
            if(!player->tag.value("InvokeTuntian", false).toBool())
                return false;
            player->tag.remove("InvokeTuntian");
            const Card *card = room->getTag("YiJuCard").value<CardStar>();
            if(!card)
                return false;
            if(!YJmateng->askForSkillInvoke(objectName()))
                return false;

            room->moveCardTo(card, NULL, Player::DrawPile, true);
            LogMessage log;
            log.type = "$YiJuLog";
            log.from = YJmateng;
            log.card_str = card->toString();
            room->sendLog(log);
            room->removeTag("YiJuCard");
        }
        return false;
    }
};

YJZhuanXiangCard::YJZhuanXiangCard(){
    will_throw = false;
}

bool YJZhuanXiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select->isKongcheng())
        return false;
    else if(to_select->isLord())
        return false;
    else if(targets.length() >= 3)
        return false;
    else
        return true;
}

void YJZhuanXiangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    const Card *card = Sanguosha->getCard(this->getSubcards().first());
    int i;
    for(i=0;i<targets.length();i++){
        ServerPlayer *target = targets.at(i);
        if(source->pindian(target, "zhuanxiang", card)){
            room->setTag("ZhuanXiangTarget"+QString::number(i), QVariant::fromValue(target));
            QString old_kingdom = target->getKingdom();
            QString kingdom = room->askForChoice(source, "zhuanxiang", "wei+shu+wu+qun");
            room->setPlayerProperty(target, "kingdom", QVariant::fromValue(kingdom));

            LogMessage log;
            log.type = "#ChangeKingdom";
            log.from = source;
            log.to << target;
            log.arg = old_kingdom;
            log.arg2 = kingdom;
            room->sendLog(log);

            source->drawCards(1);
        }
        room->throwCard(this);
    }
}

class YJZhuanXiangViewAsSkill: public OneCardViewAsSkill{
public:
    YJZhuanXiangViewAsSkill():OneCardViewAsSkill("zhuanxiang"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YJZhuanXiangCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new YJZhuanXiangCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class YJZhuanXiang: public TriggerSkill{
public:
    YJZhuanXiang():TriggerSkill("zhuanxiang"){
        events << PhaseChange;
        view_as_skill = new YJZhuanXiangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *YJchengyu, QVariant &data) const{
        Room *room = YJchengyu->getRoom();

        if(event == PhaseChange && YJchengyu->getPhase() == Player::Start){
            int i;
            for(i=0;i<3;i++){
                ServerPlayer *target = room->getTag("ZhuanXiangTarget"+QString::number(i)).value<PlayerStar>();
                if(target){
                    QString kingdom = target->getGeneral()->getKingdom();
                    room->setPlayerProperty(target, "kingdom", QVariant::fromValue(kingdom));
                    room->removeTag("ZhuanXiangTarget"+QString::number(i));
                }else
                    break;
            }
            return false;
        }
        return false;
    }
};

class YJGangDuan: public TriggerSkill{
public:
    YJGangDuan():TriggerSkill("gangduan"){
        events << Pindian;
        frequency = Frequent;
    }

    int getKingdoms(ServerPlayer *player) const{
        QSet<QString> kingdom_set;
        Room *room = player->getRoom();
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            kingdom_set << p->getKingdom();
        }

        return kingdom_set.size();
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *YJchengyu = room->findPlayerBySkillName(objectName());
        if(!YJchengyu)
            return false;
        if(!YJchengyu->askForSkillInvoke(objectName()))
            return false;
        PindianStar pindian = data.value<PindianStar>();
        QStringList choices;
        choices << pindian->from->getGeneralName()
                << pindian->to->getGeneralName()
                << "cancel";

        LogMessage log;
        log.type = "$GangDuanLog_from";
        log.from = pindian->from;
        log.to << pindian->to;
        log.card_str = pindian->from_card->getEffectIdString();
        room->sendLog(log);
        log.type = "$GangDuanLog_to";
        log.from = YJchengyu;
        log.card_str = pindian->to_card->getEffectIdString();
        room->sendLog(log);

        QString choice = room->askForChoice(YJchengyu, objectName(), choices.join("+"));
        if(choice == "cancel"){
            log.type = "$GangDuanLog_cancel";
            room->sendLog(log);
            return false;
        }

        ServerPlayer *target = YJchengyu;
        if(choice == pindian->from->getGeneralName())
            target = pindian->from;
        else
            target = pindian->to;

        const Card *old_card = target == pindian->from ? pindian->from_card : pindian->to_card;
        int num = old_card->getNumber() + 4 - getKingdoms(YJchengyu);
        Card *new_card = Sanguosha->cloneCard(old_card->objectName(), old_card->getSuit(), num);
        new_card->setSkillName(objectName());
        new_card->addSubcard(old_card);
        if(target == pindian->from)
            pindian->from_card = new_card;
        else
            pindian->to_card = new_card;

        LogMessage log2;
        log2.type = "#GangDuanLog";
        log2.from = YJchengyu;
        log2.to << target;
        log2.arg = QString::number(4 - getKingdoms(YJchengyu));
        room->sendLog(log2);

        data = QVariant::fromValue(pindian);
        return false;

    }
};

YJHuiZeCard::YJHuiZeCard(){

}

bool YJHuiZeCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;
    else
        return targets.isEmpty();
}

void YJHuiZeCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QString choice = room->askForChoice(effect.from, "huize", "drawxcard+discardxcard");
    if(choice == "drawxcard"){
        room->loseHp(effect.to, 1);
        int x = effect.from->getLostHp();
        effect.to->drawCards(x);
    }else{
        RecoverStruct recover;
        recover.who = effect.to;
        recover.card = NULL;
        recover.recover = 1;
        room->recover(effect.to, recover, true);
        int x = effect.from->getLostHp();
        if(effect.to->getCards("he").length() <= x)
            effect.to->throwAllCards();
        else
            room->askForDiscard(effect.to, "huize", x, false, true);
    }
}

class YJHuiZeViewAsSkill: public ZeroCardViewAsSkill{
public:
    YJHuiZeViewAsSkill():ZeroCardViewAsSkill("huize"){
    }

    virtual const Card *viewAs() const{
        return new YJHuiZeCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@huize";
    }
};

class YJHuiZe:public PhaseChangeSkill{
public:
    YJHuiZe():PhaseChangeSkill("huize"){
        view_as_skill = new YJHuiZeViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *YJbulianshi) const{
        if(YJbulianshi->getPhase() == Player::Start){
            Room *room = YJbulianshi->getRoom();
            if(YJbulianshi->isWounded()){
                room->askForUseCard(YJbulianshi, "@@huize", "@huize");
                room->playSkillEffect("huize", 1);
                return false;
            }else
                return false;
        }
        return false;
    }
};

class YJShuYi: public OneCardViewAsSkill{
public:
    YJShuYi():OneCardViewAsSkill("shuyi"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Slash") && to_select->getFilteredCard()->isRed();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWounded();
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("peach+analeptic") or pattern == "peach";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Peach *peach = new Peach(first->getSuit(), first->getNumber());
        peach->addSubcard(first->getId());
        peach->setSkillName(objectName());
        return peach;
    }
};

YJTanChaCard::YJTanChaCard(){

}

bool YJTanChaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select->isKongcheng() || to_select == Self)
        return false;
    else
        return targets.isEmpty();
}

void YJTanChaCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QList<int> hcids;
    foreach(const Card *card, effect.to->getHandcards()){
        int id = card->getEffectiveId();
        hcids << id;
    }
    room->fillAG(hcids, effect.from);
    int cdid = room->askForAG(effect.from, hcids, true, "tancha");
    effect.from->invoke("clearAG");
    if(cdid != -1){
        QList<ServerPlayer *> tos = room->getOtherPlayers(effect.from);
        tos.removeOne(effect.to);
        ServerPlayer *to = room->askForPlayerChosen(effect.from, tos, "tancha");
        room->showCard(effect.to, cdid, to);
    }
}

class YJTanCha: public ZeroCardViewAsSkill{
public:
    YJTanCha():ZeroCardViewAsSkill("tancha"){
    }

    virtual const Card *viewAs() const{
        return new YJTanChaCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YJTanChaCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  false;
    }
};

class YJZhaXiang: public TriggerSkill{
public:
    YJZhaXiang():TriggerSkill("zhaxiang"){
        events << CardUsed;
        frequency = NotFrequent;
    }

    QString getColor(const Card *card) const{
        if(card->isBlack())
            return "black";
        else
            return "red";
    }

    virtual bool triggerable(const ServerPlayer *target) const{

        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *YJzhoufang = room->findPlayerBySkillName(objectName());
        if(!YJzhoufang)
            return false;

        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Slash") && use.from->inMyAttackRange(YJzhoufang)){
            if(YJzhoufang->isKongcheng())
                return false;
            if(!YJzhoufang->askForSkillInvoke(objectName(), data))
                return false;
            QString prompt = QString("@zhaxiang:%1::%2").arg(use.from->getGeneralName()).arg(getColor(use.card));
            if(room->askForCard(YJzhoufang, QString(".|.|.|hand|%1").arg(getColor(use.card)), prompt, data)){
                room->throwCard(use.card);
                foreach(ServerPlayer *p, use.to){
                    if(p->isNude())
                        continue;
                    else{
                        int id = room->askForCardChosen(use.from, p, "hej", objectName());
                        room->moveCardTo(Sanguosha->getCard(id), NULL, Player::DiscardedPile, true);
                    }
                }
                return true;
            }
            return false;
        }else
            return false;
        return false;
    }
};

class YJBianCai: public TriggerSkill{
public:
    YJBianCai():TriggerSkill("biancai"){
        events << CardDiscarded ;
        frequency = NotFrequent;
    }

    virtual int getPriority() const{
        return 0;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *YJcaiyong = room->findPlayerBySkillName(objectName());
        if(!YJcaiyong)
            return false;

        if(event == CardDiscarded && player->getPhase() == Player::Discard){
            const Card *card = data.value<CardStar>();
            if(card->subcardsLength() == 0)
                return false;

            if(!YJcaiyong->askForSkillInvoke(objectName(), data))
                return false;
            room->loseHp(YJcaiyong, 1);
            if(!YJcaiyong->isAlive())
                return false;

            int i, sum = 0;
            QList<int> discard_ids = card->getSubcards();
            for(i=0; i<card->subcardsLength(); i++){
                room->fillAG(discard_ids, YJcaiyong);
                int id = room->askForAG(YJcaiyong, discard_ids, true, objectName());
                if(id == -1){
                    YJcaiyong->invoke("clearAG");
                    break;
                }else{
                    YJcaiyong->invoke("clearAG");
                    discard_ids.removeOne(id);
                    room->moveCardTo(Sanguosha->getCard(id), YJcaiyong, Player::Hand, true);
                    sum = sum + Sanguosha->getCard(id)->getNumber();
                }
            }
            if(sum/2 == (sum+1)/2 && sum != 0){
                RecoverStruct recover;
                recover.card = NULL;
                recover.who = YJcaiyong;
                recover.recover = 1;
                room->recover(YJcaiyong, recover, true);
                return false;
            }
            return false;
        }
        return false;
    }
};

YJLingYinCard::YJLingYinCard(){
    target_fixed = true;

}

void YJLingYinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    source->addToPile("LingYinPile", this->getSubcards().first(), false);
}

class YJLingYinViewAsSkill: public OneCardViewAsSkill{
public:
    YJLingYinViewAsSkill():OneCardViewAsSkill("lingyin"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Slash");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@lingyin-slash";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new YJLingYinCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class YJLingYin: public TriggerSkill{
public:
    YJLingYin():TriggerSkill("lingyin"){
        events << PhaseChange << CardEffected;
        view_as_skill = new YJLingYinViewAsSkill;
        frequency = NotFrequent;
    }

    QString getColor(const Card *card) const{
        if(card->isBlack())
            return "black";
        else
            return "red";
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *YJcaiyong, QVariant &data) const{
        Room *room = YJcaiyong->getRoom();
        if(event == PhaseChange && YJcaiyong->getPhase() == Player::Finish && !YJcaiyong->isKongcheng()){
            if(YJcaiyong->getPile("LingYinPile").length() > 1)
                return false;
            QString prompt;
            if(YJcaiyong->getPile("LingYinPile").length() == 1)
                prompt = QString("@lingyin-had:::%1").arg(getColor(Sanguosha->getCard(YJcaiyong->getPile("LingYinPile").first())));
            else
                prompt = "@lingyin-non";
            room->askForUseCard(YJcaiyong, "@@lingyin-slash", prompt);
            return false;
        }else if(event == CardEffected){
            if(YJcaiyong->getPile("LingYinPile").length() == 0)
                return false;
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(!effect.card->inherits("Slash"))
                return false;
            QList<int> yins = YJcaiyong->getPile("LingYinPile");
            const Card *samecolor = NULL;
            foreach(int id, yins){
                const Card *card = Sanguosha->getCard(id);
                if(effect.card->sameColorWith(card)){
                    samecolor = card;
                    break;
                }
            }
            if(samecolor){
                if(!YJcaiyong->askForSkillInvoke(objectName(), data))
                    return false;
                room->throwCard(samecolor);
                room->setEmotion(effect.from, "bad");
                return true;
            }else
                return false;
        }
        return false;
    }
};

YJYangXiCard::YJYangXiCard(){

}

bool YJYangXiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void YJYangXiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.to->turnOver();
    effect.from->turnOver();

    RecoverStruct recover;
    recover.card = NULL;
    recover.who = effect.from;
    recover.recover = 1;
    if(effect.from->isWounded())
        room->recover(effect.from, recover, true);
    else if(effect.to->isWounded())
        recover.who = effect.to;
        room->recover(effect.to, recover, true);
}

class YJYangXi: public ZeroCardViewAsSkill{
public:
    YJYangXi():ZeroCardViewAsSkill("yangxi"){
    }

    virtual const Card *viewAs() const{
        return new YJYangXiCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YJYangXiCard") && player->isWounded();
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }
};

class YJLianRen: public ProhibitSkill{
public:
    YJLianRen():ProhibitSkill("lianren"){
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if (to->hasSkill("lianren") && !to->faceUp())
            return (card->isNDTrick());
        else{
            return false;
        }
    }
};

class YJLianRenEffect: public TriggerSkill{
public:
    YJLianRenEffect():TriggerSkill("#lianren-effect"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("lianren") && !target->faceUp();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("GodSalvation") || effect.card->inherits("AmazingGrace"))
            return true;

        return false;
    }
};

YJZhengLveCard::YJZhengLveCard(){
    target_fixed = false;
}

int YJZhengLveCard::getSuits(ServerPlayer *player) const{
    QSet<QString> suit_set;
    foreach(const Card *cd, player->getHandcards()){
        suit_set << cd->getSuitString();
    }
    return suit_set.size();
}

bool YJZhengLveCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select->isKongcheng())
        return false;
    else
        return targets.isEmpty();
}

void YJZhengLveCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    foreach(const Card *cd, source->getHandcards()){
        room->showCard(source, cd->getEffectiveId());
    }

    LogMessage log;
    log.type = "#ZhengLveLog";
    log.from = source;
    log.arg = QString::number(getSuits(source));
    room->sendLog(log);

    ServerPlayer *to = targets.first();
    QString choice = room->askForChoice(to, "zhenglve", "showcard+askfrom");
    if(choice == "showcard"){
        log.type = "#ZhengLveSelected";
        log.from = to;
        log.arg = choice;
        room->sendLog(log);

        foreach(const Card *cd, to->getHandcards()){
            room->showCard(to, cd->getEffectiveId());
        }

        log.type = "#ZhengLveLog";
        log.from = to;
        log.arg = QString::number(getSuits(to));
        room->sendLog(log);

        if(getSuits(to) <= getSuits(source)){
            DamageStruct damage;
            damage.card = NULL;
            damage.from = source;
            damage.to = to;
            damage.damage = 1;
            room->damage(damage);
        }
    }else{
        log.type = "#ZhengLveSelected";
        log.from = to;
        log.arg = choice;
        room->sendLog(log);

        QString choice2 = room->askForChoice(source, "zhenglve", "getcard+onedamage");
        if(choice2 == "getcard"){
            int card_id = room->askForCardChosen(source, to, "hej", "zhenglve");
            room->moveCardTo(Sanguosha->getCard(card_id), source, Player::Hand, room->getCardPlace(card_id) == Player::Hand ? false : true);
        }else{
            DamageStruct damage;
            damage.card = NULL;
            damage.from = source;
            damage.to = to;
            damage.damage = 1;
            room->damage(damage);
        }
    }
}

class YJZhengLve: public ZeroCardViewAsSkill{
public:
    YJZhengLve():ZeroCardViewAsSkill("zhenglve"){
    }

    virtual const Card *viewAs() const{
        return new YJZhengLveCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YJZhengLveCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }
};

class YJZhengShang: public TriggerSkill{
public:
    YJZhengShang():TriggerSkill("zhengshang"){
        events << PhaseChange << CardAsked << Death;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

   virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *YJyuwenze = room->findPlayerBySkillName(objectName());
        if(!YJyuwenze)
            return false;
        if(event == PhaseChange && YJyuwenze->getPhase() == Player::Discard){
            while(YJyuwenze->getHandcardNum() > YJyuwenze->getHp()){
                QList<ServerPlayer *> tos;
                foreach(ServerPlayer *p, room->getOtherPlayers(YJyuwenze)){
                    if(p->getPile("ZhengShangPile").isEmpty())
                        tos << p;
                }
                if(tos.isEmpty())
                    return false;
                if(!YJyuwenze->askForSkillInvoke(objectName(), data))
                    return false;
                ServerPlayer *to = room->askForPlayerChosen(YJyuwenze, tos, objectName());
                const Card *card = room->askForCard(YJyuwenze, ".|.|.|hand|.", "@zhengshang:"+to->getGeneralName());
                to->addToPile("ZhengShangPile", card->getEffectiveId(), true);
            }
            return false;
        }
        if(event == CardAsked){
            if(player->getPile("ZhengShangPile").isEmpty())
                return false;
            QString pattern = data.toString();
            if(pattern == "jink"){
                if(!YJyuwenze->askForSkillInvoke(objectName(), data))
                    return false;
                room->throwCard(player->getPile("ZhengShangPile").first());
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->provide(jink);
                room->setEmotion(player, "good");
                return true;
            }else
                return false;
        }else if(event == PhaseChange
                 && player->getPhase() == Player::Start
                 && !player->getPile("ZhengShangPile").isEmpty()
                 && player->getJudgingArea().length() != 0){
            if(!YJyuwenze->askForSkillInvoke(objectName(), data))
                return false;
            room->throwCard(player->getPile("ZhengShangPile").first());
            foreach(const Card *cd, player->getJudgingArea()){
                room->throwCard(cd);
            }
            room->setEmotion(player, "good");
            return false;
        }else if(event == Death){
            if(player == YJyuwenze){
                foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                    if(!p->getPile("ZhengShangPile").isEmpty()){
                        room->throwCard(player->getPile("ZhengShangPile").first());
                    }
                }
                return false;
            }else
                return false;
        }
        return false;
    }
};
/*
class YJTianHui: public TriggerSkill{
public:
    YJTianHui():TriggerSkill("tianhui"){
        events << PhaseChange ;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

   virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == PhaseChange && player->getPhase() == Player::Start){
            const Card *card = room->askForCardShow(player, player, objectName());
            Card::Suit suit = card->getSuit();

        }
        return false;
    }
};

class YJJiFeng: public TriggerSkill{
public:
    YJJiFeng():TriggerSkill("jifeng"){
        events << Damaged ;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

   virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(!player->isAlive() || player->isKongcheng())
            return false;
        if(!player->askForSkillInvoke(objectName(), data))
            return false;
        const Card *card = room->askForCardShow(player, player, objectName());
        DamageStruct damage = data.value<DamageStruct>();
        foreach(const Card *cd, damage.from->getHandcards()){
            if(card->sameColorWith(cd))
                room->moveCardTo(cd, NULL, Player::DiscardedPile, true);
            else
                room->showCard(damage.from, cd->getEffectiveId());
        }
    }
};
*/
ZhangQi::ZhangQi(Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("zhang_qi");
}

void ZhangQi::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *card = room->askForCard(effect.to, "Peach,Analeptic|.|.|.|.", "@zhangqi:" + effect.from->objectName());
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
    if(effect.to->getCards("h").length() <= 4)
        effect.to->throwAllHandCards();
    else
        room->askForDiscard(effect.to, "yuqinguzong", 4, false, false);
}

class RedFlagSkill: public WeaponSkill{
public:
    RedFlagSkill():WeaponSkill("red_flag"){
        events << CardUsed;

    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->inherits("Slash"))
            return false;
        const Card *card = use.card;
        Card::Suit suit = Card::NoSuit;
        if(card->getSuit() == Card::Club)
            suit = Card::Diamond;
        else if(card->getSuit() == Card::Spade)
            suit = Card::Heart;
        Card *slash = new Slash(suit, card->getNumber());
        slash->addSubcard(card->getId());
        use.card = slash;
        data = QVariant::fromValue(use);
        Room *room = player->getRoom();
        LogMessage log;
        log.type = "#RedFlagLog";
        log.from = player;
        room->sendLog(log);
        return false;
    }
};

RedFlag::RedFlag(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("red_flag");
    skill = new RedFlagSkill;
}

JieJian::JieJian(Suit suit, int number):Disaster(suit, number){
    setObjectName("jiejian");

    judge.pattern = QRegExp("(.*):(club):([2-9])");
    judge.good = false;
    judge.reason = objectName();
}

void JieJian::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();
    foreach(ServerPlayer *p, room->getOtherPlayers(target)){
        const Card *slash = room->askForCard(p, "slash", "@jiejian-slash:"+target->getGeneralName());
        if(slash){
            room->cardEffect(slash, p, target);
            if(target->isAlive())
                room->moveCardTo(slash, target, Player::Hand, true);
        }
    }
}

YJ1stPackage::YJ1stPackage()
    :Package("YJ1st")
{
    General *YJchengyu = new General(this, "YJchengyu", "wei", 3);
    YJchengyu->addSkill(new YJZhuanXiang);
    YJchengyu->addSkill(new YJGangDuan);

    General *YJyuwenze = new General(this, "YJyuwenze", "wei");
    YJyuwenze->addSkill(new YJZhengShang);

    General *YJzhangyi = new General(this, "YJzhangyi", "shu");
    YJzhangyi->addSkill(new YJZhengLve);

    General *YJxingcai = new General(this, "YJxingcai", "shu", 3, false);
    YJxingcai->addSkill(new YJYangXi);
    YJxingcai->addSkill(new YJLianRen);
    YJxingcai->addSkill(new YJLianRenEffect);

    related_skills.insertMulti("lianren", "#lianren-effect");

    General *YJbulianshi = new General(this, "YJbulianshi", "wu", 3, false);
    YJbulianshi->addSkill(new YJHuiZe);
    YJbulianshi->addSkill(new YJShuYi);

    General *YJzhoufang = new General(this, "YJzhoufang", "wu", 3);
    YJzhoufang->addSkill(new YJTanCha);
    YJzhoufang->addSkill(new YJZhaXiang);

    General *YJcaiyong = new General(this, "YJcaiyong", "qun", 3);
    YJcaiyong->addSkill(new YJBianCai);
    YJcaiyong->addSkill(new YJLingYin);

    General *YJmateng = new General(this, "YJmateng", "qun", 4);
    YJmateng->addSkill(new YJXiJun);
    YJmateng->addSkill(new YJYiJu);

    addMetaObject<YJZhuanXiangCard>();
    addMetaObject<YJHuiZeCard>();
    addMetaObject<YJTanChaCard>();
    addMetaObject<YJLingYinCard>();
    addMetaObject<YJYangXiCard>();
    addMetaObject<YJZhengLveCard>();

}

YJ1stCardPackage::YJ1stCardPackage()
    :Package("YJ1st_Card")
{
    QList<Card *> cards;

    cards << new ZhangQi(Card::Club, 3)
          << new YuQinGuZong(Card::Heart, 11)
          << new RedFlag(Card::Heart, 12)
          << new JieJian(Card::Club, 1);


    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(YJ1st)
ADD_PACKAGE(YJ1stCard)
