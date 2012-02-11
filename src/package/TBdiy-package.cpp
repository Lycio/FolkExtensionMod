#include "TBdiy-package.h"
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

class DiyXueLu: public TriggerSkill{
public:
    DiyXueLu():TriggerSkill("diyxuelu"){
        events << SlashEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *skillowner = room->findPlayerBySkillName(objectName());
        if(!skillowner)
            return false;
        if(skillowner->getPhase() != Player::NotActive)
            return false;
        if(!skillowner->inMyAttackRange(player))
            return false;
        if(!skillowner->askForSkillInvoke(objectName(), data))
            return false;
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!room->askForCard(skillowner, "Horse|.|.|.|.", "@diyxuelu-horse:"+effect.to->getGeneralName(), data)){
            room->loseHp(skillowner, 1);
            room->playSkillEffect("diyxuelu", 1);
            room->moveCardTo(effect.slash, skillowner, Player::Hand, true);
        }
        room->playSkillEffect("diyxuelu", 2);
        return true;
    }
};

DiyShenZhiCard::DiyShenZhiCard(){
    once = true;
    will_throw = false;
}

bool DiyShenZhiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;
    else
        return targets.isEmpty();
}

void DiyShenZhiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *card = Sanguosha->getCard(this->subcards.first());
    room->showCard(effect.from, card->getEffectiveId(), effect.to);
    QString choice = room->askForChoice(effect.to, "diyshenzhi", "obtain+reject");
    if(choice == "obtain"){
        room->moveCardTo(card, effect.to, Player::Hand, false);
        effect.to->drawCards(1);
        room->loseHp(effect.to, 1);
        room->playSkillEffect("diyshenzhi", 1);
    }else{
        room->throwCard(this);
        int card_id = room->askForCardChosen(effect.from, effect.to, "he", "diyshenzhi");
        const Card *card = Sanguosha->getCard(card_id);
        bool is_public = room->getCardPlace(card_id) != Player::Hand;
        room->moveCardTo(card, effect.from, Player::Hand, is_public ? true : false);
        room->playSkillEffect("diyshenzhi", 2);
    }
}

class DiyShenZhi: public OneCardViewAsSkill{
public:
    DiyShenZhi():OneCardViewAsSkill("diyshenzhi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DiyShenZhiCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        DiyShenZhiCard *card = new DiyShenZhiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class DiyZhaoLie: public TriggerSkill{
public:
    DiyZhaoLie():TriggerSkill("diyzhaolie"){
        events << SlashEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(!player->askForSkillInvoke(objectName(), data))
            return false;
        room->playSkillEffect(objectName());
        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(.*):(.*)");
        judge.reason = objectName();
        judge.who = player;
        room->judge(judge);

        if(!judge.card->sameColorWith(effect.slash)){
            Jink *jink = new Jink(Card::NoSuit, 0);
            room->slashResult(effect, jink);
            LogMessage log;
            log.type = "#DiyZhaoLie";
            log.from = player;
            room->sendLog(log);
            room->playSkillEffect("diyzhaolie", qrand() %2 + 1);
            return true;
        }else{
            return false;
        }
        return false;
    }
};

DiyDouDanCard::DiyDouDanCard(){
    target_fixed = true;
}

void DiyDouDanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<int> cards = room->getNCards(3), left;
    room->fillAG(cards, source);
    QString choice = room->askForChoice(source, "diydoudan", "obtain1cd+guan3xing");
    if(choice == "obtain1cd"){
        int card_id = room->askForAG(source, cards, false, "diydoudan");
        room->playSkillEffect("diydoudan", 1);
        cards.removeOne(card_id);
        source->obtainCard(Sanguosha->getCard(card_id));
        foreach(int id, cards){
            room->throwCard(id);
        }        
        source->invoke("clearAG");
    }else{
        source->invoke("clearAG");
        room->playSkillEffect("diydoudan", qrand() %2 + 2);
        room->doGuanxing(source, cards, true);
    }
 }

class DiyDouDanViewAsSkill: public ZeroCardViewAsSkill{
public:
    DiyDouDanViewAsSkill():ZeroCardViewAsSkill("diydoudan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@diydoudan";
    }

    virtual const Card *viewAs() const{
        return new DiyDouDanCard;
    }
};

class DiyDouDan: public TriggerSkill{
public:
    DiyDouDan():TriggerSkill("diydoudan"){
        view_as_skill = new DiyDouDanViewAsSkill;
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == PhaseChange && player->getPhase() == Player::Draw){
            if(room->askForUseCard(player, "@@diydoudan", "@diydoudan")){
                QList<ServerPlayer *> targets;
                foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                    if(player->inMyAttackRange(p))
                        targets << p;
                    if(p->isKongcheng() && p->hasSkill("kongcheng"))
                        targets.removeOne(p);
                }
                if(targets.isEmpty()){
                    player->skip(Player::Play);
                    return true;
                }else{                           
                    ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
                    LogMessage log;
                    log.type = "#DouDanLog";
                    log.from = player;
                    log.to << target;
                    room->sendLog(log);
                    Slash *slash = new Slash(Card::NoSuit, 0);
                    room->cardEffect(slash, player, target);
                    player->skip(Player::Play);
                    return true;
                }
            }
            return false;
        }
        return false;
    }
};

DiyKuangXiCard::DiyKuangXiCard(){
}

bool DiyKuangXiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;
    else if(Self->distanceTo(to_select) > 1)
        return false;
    else if(to_select->isKongcheng())
        return false;
    else
        return targets.isEmpty();
}

void DiyKuangXiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->throwCard(this);

    int id = room->askForCardChosen(effect.from, effect.to, "h", "diykuangxi");
    room->showCard(effect.to, id);
    const Card *card = Sanguosha->getCard(id);
    if(card->inherits("Slash") || card->inherits("Jink")){
        room->throwCard(card);
        if(effect.to->isKongcheng() && effect.to->hasSkill("kongcheng"))
            return;
        else{
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            room->cardEffect(slash, effect.from, effect.to);
        }
    }else
        effect.from->setFlags("KuangXiFail");
}

class DiyKuangXiViewAsSkill:public OneCardViewAsSkill{
public:
    DiyKuangXiViewAsSkill():OneCardViewAsSkill("diykuangxi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@diykuangxi";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new DiyKuangXiCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class DiyKuangXi: public TriggerSkill{
public:
    DiyKuangXi():TriggerSkill("diykuangxi"){
        events << PhaseChange;
        view_as_skill = new DiyKuangXiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == PhaseChange && player->getPhase() == Player::Start){
            player->setMark("kxused", 0);
            while(!player->hasFlag("KuangXiFail")){
                QList<ServerPlayer *> targets;
                foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                    if(player->distanceTo(p) <= 1 && !p->isKongcheng())
                        targets << p;
                }
                if(targets.isEmpty()
                        || player->isKongcheng()
                        || !player->askForSkillInvoke(objectName(), data))
                    break;
                if(room->askForUseCard(player, "@@diykuangxi", "@diykuangxi")){
                    player->addMark("kxused");
                    int x = player->getMark("kxused");
                    room->playSkillEffect("diykuangxi", qMin(x, 4));
                }
            }
            if(player->getMark("kxused") != 0){
                player->skip(Player::Judge);
                player->skip(Player::Draw);
            }
            return false;
        }
        return false;
    }
};

DiyBianZhenCard::DiyBianZhenCard(){

}

bool DiyBianZhenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void DiyBianZhenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(effect.from->getCards("he").length() < 2)
        room->loseHp(effect.from, 1);
    else{
        if(!room->askForDiscard(effect.from, "diybianzhen", 2, true, true))
            room->loseHp(effect.from, 1);
    }
    room->setTag("BianZhenTarget", QVariant::fromValue(effect.to));
    QString choice = room->askForChoice(effect.from, "diybianzhen", "start+judge+draw+discard+finish");
    effect.from->setFlags(choice);

    LogMessage log;
    log.type = "#BianZhenLog";
    log.from = effect.from;
    log.arg = choice;
    room->sendLog(log);
}

class DiyBianZhenViewAsSkill: public ZeroCardViewAsSkill{
public:
    DiyBianZhenViewAsSkill():ZeroCardViewAsSkill("diybianzhen"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DiyBianZhenCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }

    virtual const Card *viewAs() const{
        return new DiyBianZhenCard;
    }
};

class DiyBianZhen: public TriggerSkill{
public:
    DiyBianZhen():TriggerSkill("diybianzhen"){
        view_as_skill = new DiyBianZhenViewAsSkill;
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == PhaseChange && player->getPhase() == Player::Finish){
            ServerPlayer *target = room->getTag("BianZhenTarget").value<PlayerStar>();
            if(target){
                QList<Player::Phase> gainPhases;
                if(player->hasFlag("start"))
                    gainPhases << Player::Start;
                else if(player->hasFlag("judge"))
                    gainPhases << Player::Judge;
                else if(player->hasFlag("draw"))
                    gainPhases << Player::Draw;
                else if(player->hasFlag("discard"))
                    gainPhases << Player::Discard;
                else if(player->hasFlag("finish"))
                    gainPhases << Player::Finish;
                target->play(gainPhases);
            }
            return false;
        }
        return false;
    }
};

class DiyCongWen: public PhaseChangeSkill{
public:
    DiyCongWen():PhaseChangeSkill("diycongwen"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("diycongwen") == 0
                && target->getPhase() == Player::Start
                && target->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#CongWenWake";
        log.from = player;
        room->sendLog(log);

        room->playSkillEffect("diycongwen");
        room->broadcastInvoke("animate", "lightbox:$diycongwen:5000");
        room->getThread()->delay(2000);

        if(room->askForChoice(player, objectName(), "recover+draw") == "recover"){
            RecoverStruct recover;
            recover.who = player;
            room->recover(player, recover);
        }else
            room->drawCards(player, 2);

        room->setPlayerMark(player, "diycongwen", 1);
        room->acquireSkill(player, "wuyan");

        room->loseMaxHp(player);

        return false;
    }
};

DiyXianXiCard::DiyXianXiCard(){
}

bool DiyXianXiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select->hasFlag("SlashTarget") || to_select == Self)
        return false;
    else
        return true;
}

bool DiyXianXiCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return !targets.isEmpty();
}

void DiyXianXiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *slash = room->getTag("SlashCard").value<CardStar>();
    room->throwCard(slash);
    room->cardEffect(slash, effect.from, effect.to);
}

class DiyXianXiViewAsSkill: public ZeroCardViewAsSkill{
public:
    DiyXianXiViewAsSkill():ZeroCardViewAsSkill("diyxianxi"){
    }

    virtual const Card *viewAs() const{
        return new DiyXianXiCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@diyxianxi-slash";
    }
};

class DiyXianXi: public TriggerSkill{
public:
    DiyXianXi():TriggerSkill("diyxianxi"){
        events << CardUsed << SlashMissed;
        view_as_skill = new DiyXianXiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(!use.card->inherits("Slash"))
                return false;
            room->setTag("SlashCard", QVariant::fromValue(use.card));
            foreach(ServerPlayer *p, use.to){
                room->setPlayerFlag(p, "SlashTarget");
            }
            if(room->askForUseCard(player, "@@diyxianxi-slash", "@diyxianxi")){
                room->playSkillEffect("diyxianxi", qrand() %2 + 1);
            }
            return false;
        }else if(event == SlashMissed && !data.value<SlashEffectStruct>().to->hasFlag("SlashTarget")){
            if(!room->askForDiscard(player, "diyxianxi-missed", 2, true, true)){
                room->loseHp(player, 1);
                room->playSkillEffect("diyxianxi", 3);
                return false;
            }else
                room->playSkillEffect("diyxianxi", 4);
                return false;
        }else if(event == CardFinished){
            CardUseStruct use = data.value<CardUseStruct>();
            if(!use.card->inherits("Slash"))
                return false;
            foreach(ServerPlayer *p, room->getAllPlayers()){
                room->setPlayerFlag(p, "-SlashTarget");
            }
            return false;
        }
        return false;
    }
};

DiyXueJingCard::DiyXueJingCard(){
    will_throw = false;
    target_fixed = true;
}

void DiyXueJingCard::use(Room *room, ServerPlayer *from, const QList<ServerPlayer *> &targets) const{
    const Card *card = Sanguosha->getCard(this->subcards.first());
    room->moveCardTo(card, from, Player::Judging, true);
    LogMessage log;
    log.type = "$XueJingLog";
    log.from = from;
    log.card_str = card->toString();
    room->sendLog(log);
    from->drawCards(1);

    ServerPlayer *to = room->askForPlayerChosen(from, room->getOtherPlayers(from), "diyxuejing");
    from->tag["XueJingTarget"] = QVariant::fromValue(to);

    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("diyxuejing");

    CardUseStruct use;
    use.from = from;
    use.to << to;
    use.card = slash;
    room->useCard(use, false);

    room->setFixedDistance(from, to, 1);
}

class DiyXueJingViewAsSkill: public OneCardViewAsSkill{
public:
    DiyXueJingViewAsSkill():OneCardViewAsSkill("diyxuejing"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        if(to_select->getFilteredCard()->getSuit() == Card::Heart)
            return false;

        return to_select->getFilteredCard()->inherits("EquipCard") || to_select->getFilteredCard()->inherits("BasicCard");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DiyXueJingCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new DiyXueJingCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class DiyXueJing: public TriggerSkill{
public:
    DiyXueJing():TriggerSkill("diyxuejing"){
        view_as_skill = new DiyXueJingViewAsSkill;

        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == PhaseChange){
            ServerPlayer *target = player->tag["XueJingTarget"].value<PlayerStar>();
            if(player->getPhase() == Player::Finish && target){
                Room *room = player->getRoom();
                room->setFixedDistance(player, target, -1);
                player->tag.remove("XueJingTarget");
            }
        }
        return false;
    }
};

DiyJiFengCard::DiyJiFengCard(){
    will_throw = false;
}

bool DiyJiFengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select->getEquips().isEmpty())
        return false;
    else
        return targets.isEmpty();
}

void DiyJiFengCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *from = effect.from;
    ServerPlayer *to = effect.to;
    from->tag["JiFengTarget"] = QVariant::fromValue(to);
    Room *room = from->getRoom();
    from->addToPile("JiFengPile", this->subcards.first(), false);
    int card_id = room->askForCardChosen(from, to, "e", "diyjifeng");
    from->obtainCard(Sanguosha->getCard(card_id));
    room->setFixedDistance(from, to, 1);
}

class DiyJiFengViewAsSkill: public OneCardViewAsSkill{
public:
    DiyJiFengViewAsSkill():OneCardViewAsSkill("diyjifeng"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DiyJiFengCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new DiyJiFengCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class DiyJiFeng: public TriggerSkill{
public:
    DiyJiFeng():TriggerSkill("diyjifeng"){
        view_as_skill = new DiyJiFengViewAsSkill;

        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *target = player->tag["JiFengTarget"].value<PlayerStar>();

        if(event == PhaseChange){
            if(player->getPhase() == Player::Finish && target){
                Room *room = player->getRoom();
                room->setFixedDistance(player, target, -1);
                player->tag.remove("JiFengTarget");
            }
        }
        return false;
    }
};

class DiyJingCao: public TriggerSkill{
public:
    DiyJingCao():TriggerSkill("diyjingcao"){

        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == PhaseChange && player->getPhase() == Player::Draw){
            QList<int> zhoupile = player->getPile("JiFengPile");
            if(zhoupile.isEmpty())
                return false;
            else{
                if(!player->askForSkillInvoke(objectName(), data))
                    return false;
                foreach(int id, zhoupile){
                    const Card *card = Sanguosha->getCard(id);
                    room->moveCardTo(card, player, Player::Hand, false);
                }
                room->playSkillEffect("diyjingcao", qrand() %2 + 1);
                return true;
            }
        }
        return false;
    }
};

class DiyXueZhan: public TriggerSkill{
public:
    DiyXueZhan():TriggerSkill("diyxuezhan"){
        events << PhaseChange << Damage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;//target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *skillowner = room->findPlayerBySkillName(objectName());
        if(!skillowner)
            return false;

        if(event == PhaseChange && skillowner->getPhase() == Player::Draw){
            if(!skillowner->askForSkillInvoke(objectName(), data))
                return false;
            ServerPlayer *to = room->askForPlayerChosen(skillowner, room->getOtherPlayers(skillowner), objectName());
            ServerPlayer *from = skillowner;

            Duel *duel = new Duel(Card::NoSuit, 0);
            duel->setSkillName(objectName());
            duel->setCancelable(false);

            CardUseStruct use;
            use.from = from;
            use.to << to;
            use.card = duel;
            room->useCard(use);

            room->playSkillEffect("diyxuezhan", qrand() %2 + 1);
            return true;
        }else if(event == Damage){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.card && damage.card->getSkillName() == objectName()){
                if(damage.from == skillowner){
                    skillowner->drawCards(2);
                    room->acquireSkill(skillowner, "paoxiao", true);
                }else
                    room->setPlayerFlag(skillowner, "xianzhen_failed");
            }
            return false;
        }else if(event == PhaseChange && skillowner->getPhase() == Player::Finish){
            if(skillowner->hasSkill("paoxiao"))
                room->detachSkillFromPlayer(skillowner, "paoxiao");
            return false;
        }
        return false;
    }
};

class DiyMingZhi: public TriggerSkill{
public:
    DiyMingZhi():TriggerSkill("diymingzhi"){
        events << SlashEffect << SlashHit << SlashMissed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == SlashEffect){
            if(!player->askForSkillInvoke(objectName(), data))
                return false;
            QString choice = room->askForChoice(player, objectName(), "hit+missed");
            player->setFlags(choice);
            LogMessage log;
            log.type = "#MingZhiLog";
            log.from = player;
            log.arg = choice;
            room->sendLog(log);
            room->playSkillEffect("diymingzhi", qrand() %2 + 1);
        }else if(event == SlashHit && player->hasFlag("hit")){
            player->setFlags("-hit");
            player->drawCards(1);
        }else if(event == SlashMissed && player->hasFlag("missed")){
            player->setFlags("-missed");
            player->drawCards(1);
        }
        return false;
    }
};

class DiyChiShen: public TriggerSkill{
public:
    DiyChiShen():TriggerSkill("diychishen"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == PhaseChange && player->getPhase() == Player::Finish){
            if(!player->isKongcheng())
                return false;
            if(!player->askForSkillInvoke(objectName(), data))
                return false;
            player->getRoom()->playSkillEffect("diychishen");
            player->drawCards(1);
        }
        return false;
    }
};

DiyXiXueCard::DiyXiXueCard(){
    will_throw = true;
}

DiyXiXueDialog *DiyXiXueDialog::GetInstance(){
    static DiyXiXueDialog *instance;
    if(instance == NULL)
        instance = new DiyXiXueDialog;

    return instance;
}

DiyXiXueDialog::DiyXiXueDialog()
{
    setWindowTitle(tr("diyxixue"));

    group = new QButtonGroup(this);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(createLeft());
    layout->addWidget(createRight());

    setLayout(layout);

    connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(selectCard(QAbstractButton*)));
}

void DiyXiXueDialog::popup(){
    foreach(QAbstractButton *button, group->buttons()){
        const Card *card = map[button->objectName()];
        button->setEnabled(card->isAvailable(Self));
    }

    Self->tag.remove("DiyXiXue");
    exec();
}

void DiyXiXueDialog::selectCard(QAbstractButton *button){
    CardStar card = map.value(button->objectName());
    Self->tag["DiyXiXue"] = QVariant::fromValue(card);

    accept();
}

QGroupBox *DiyXiXueDialog::createLeft(){
    QGroupBox *box = new QGroupBox;
    box->setTitle(tr("Basic cards"));

    QVBoxLayout *layout = new QVBoxLayout;

    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->getTypeId() == Card::Basic && !map.contains(card->objectName())){
            Card *c = Sanguosha->cloneCard(card->objectName(), Card::NoSuit, 0);
            c->setParent(this);

            layout->addWidget(createButton(c));
        }
    }

    layout->addStretch();

    box->setLayout(layout);
    return box;
}

QGroupBox *DiyXiXueDialog::createRight(){
    QGroupBox *box = new QGroupBox(tr("Non delayed tricks"));
    QHBoxLayout *layout = new QHBoxLayout;

    QGroupBox *box1 = new QGroupBox(tr("Single target"));
    QVBoxLayout *layout1 = new QVBoxLayout;

    QGroupBox *box2 = new QGroupBox(tr("Multiple targets"));
    QVBoxLayout *layout2 = new QVBoxLayout;


    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->isNDTrick() && !map.contains(card->objectName())){
            Card *c = Sanguosha->cloneCard(card->objectName(), Card::NoSuit, 0);
            c->setSkillName("diyxixue");
            c->setParent(this);

            QVBoxLayout *layout = c->inherits("SingleTargetTrick") ? layout1 : layout2;
            layout->addWidget(createButton(c));
        }
    }

    box->setLayout(layout);
    box1->setLayout(layout1);
    box2->setLayout(layout2);

    layout1->addStretch();
    layout2->addStretch();

    layout->addWidget(box1);
    layout->addWidget(box2);
    return box;
}

QAbstractButton *DiyXiXueDialog::createButton(const Card *card){
    QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(card->objectName()));
    button->setObjectName(card->objectName());
    button->setToolTip(card->getDescription());

    map.insert(card->objectName(), card);
    group->addButton(button);

    return button;
}

bool DiyXiXueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    CardStar card = Self->tag.value("DiyXiXue").value<CardStar>();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card);
}

bool DiyXiXueCard::targetFixed() const{
    CardStar card = Self->tag.value("DiyXiXue").value<CardStar>();
    return card && card->targetFixed();
}

bool DiyXiXueCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    CardStar card = Self->tag.value("DiyXiXue").value<CardStar>();
    return card && card->targetsFeasible(targets, Self);
}

const Card *DiyXiXueCard::validate(const CardUseStruct *card_use) const{
    Room *room = card_use->from->getRoom();
    room->playSkillEffect("diyxixue", qrand() %4 + 1);
    Card *use_card = Sanguosha->cloneCard(user_string, Card::NoSuit, 0);
    use_card->setSkillName("diyxixue");
    room->throwCard(this);
    return use_card;
}

class DiyXiXue: public ViewAsSkill{
public:
    DiyXiXue():ViewAsSkill("diyxixue"){
    }

    virtual QDialog *getDialog() const{
        return DiyXiXueDialog::GetInstance();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() < Self->getHandcardNum())
            return NULL;

        CardStar c = Self->tag.value("DiyXiXue").value<CardStar>();
        if(c){
            DiyXiXueCard *card = new DiyXiXueCard;
            card->setUserString(c->objectName());
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->getHandcardNum() < 2)
            return false;
        else
            return !player->hasUsed("DiyXiXueCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  false;
    }
};

TBdiyPackage::TBdiyPackage()
    :Package("TBdiy")
{
    General *diycaoang = new General(this, "diycaoang", "wei", 4);
    diycaoang->addSkill(new DiyXueLu);

    General *diyganqian = new General(this, "diyganqian", "shu", 3, false);
    diyganqian->addSkill(new DiyShenZhi);
    diyganqian->addSkill(new DiyZhaoLie);

    General *diyjiangwei = new General(this, "diyjiangwei", "shu", 4, true);
    diyjiangwei->addSkill(new DiyDouDan);

    General *diyweiyan = new General(this, "diyweiyan", "shu", 4, true);
    diyweiyan->addSkill(new DiyKuangXi);

    General *diyxushu = new General(this, "diyxushu", "shu", 4, true);
    diyxushu->addSkill(new DiyBianZhen);
    diyxushu->addSkill(new DiyCongWen);
    related_skills.insertMulti("diycongwen", "wuyan");

    General *diyhandang = new General(this, "diyhandang", "wu", 4);
    diyhandang->addSkill(new DiyXianXi);

    General *diydingfeng = new General(this, "diydingfeng", "wu", 4);
    diydingfeng->addSkill(new DiyXueJing);

    General *diylingcao = new General(this, "diylingcao", "wu", 4);
    diylingcao->addSkill(new DiyJiFeng);
    diylingcao->addSkill(new DiyJingCao);

    General *diyhuaxiong = new General(this, "diyhuaxiong", "qun", 4);
    diyhuaxiong->addSkill(new DiyXueZhan);

    General *diymushun = new General(this, "diymushun", "qun", 4);
    diymushun->addSkill(new DiyMingZhi);

    General *diymiheng = new General(this, "diymiheng", "qun", 3);
    diymiheng->addSkill(new DiyChiShen);
    diymiheng->addSkill(new DiyXiXue);

    addMetaObject<DiyXianXiCard>();
    addMetaObject<DiyXiXueCard>();
    addMetaObject<DiyShenZhiCard>();
    addMetaObject<DiyKuangXiCard>();
    addMetaObject<DiyXueJingCard>();
    addMetaObject<DiyDouDanCard>();
    addMetaObject<DiyJiFengCard>();
    addMetaObject<DiyBianZhenCard>();
}

ADD_PACKAGE(TBdiy)
