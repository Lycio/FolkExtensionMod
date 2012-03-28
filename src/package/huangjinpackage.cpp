#include "huangjinpackage.h"
#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "standard-skillcards.h"

class Rengong: public PhaseChangeSkill{
public:
    Rengong():PhaseChangeSkill("rengong"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *zhangliang) const{
        Room *room = zhangliang->getRoom();

        if(zhangliang->getPhase() != Player::Draw)
            return false;

        QList<ServerPlayer *> players = room->getOtherPlayers(zhangliang);
        foreach(ServerPlayer *p, players){
            if(p->isKongcheng())
                players.removeOne(p);
        }

        if(players.isEmpty() || !zhangliang->askForSkillInvoke(objectName()))
            return false;

        room->playSkillEffect(objectName());

        ServerPlayer *rengonger = room->askForPlayerChosen(zhangliang, players, objectName());
        QList<int> handcards = rengonger->handCards();

        room->fillAG(handcards, zhangliang);
        int card_id = room->askForAG(zhangliang, handcards, false, objectName());
        room->moveCardTo(Sanguosha->getCard(card_id), zhangliang, Player::Hand, false);
        room->broadcastInvoke("clearAG");

        return true;
    }
};

class Shishi: public TriggerSkill{
public:
    Shishi():TriggerSkill("shishi"){
        events << Death;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom ();
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        int nheal = 0;
        if(killer && killer->hasSkill(objectName()))
            nheal = killer->getMaxHP() - killer->getHp();

        if(nheal > 0 && room->askForSkillInvoke(killer, objectName())){
            RecoverStruct recover;
            recover.who = killer;
            recover.recover = nheal;
            room->recover(killer, recover);
            room->playSkillEffect(objectName());
        }

        return false;
    }
};

class Feiyan: public TriggerSkill{
public:
    Feiyan():TriggerSkill("feiyan"){
        events << CardEffected;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        ServerPlayer *zhangyan = room->findPlayerBySkillName(objectName());
        if(effect.card == NULL
           || !effect.card->inherits("Slash")
           || (effect.from->hasSkill("kongcheng") && effect.from->isKongcheng())
           || effect.from == zhangyan
           || zhangyan->getPhase() != Player::NotActive)
            return false;

        if(zhangyan->inMyAttackRange(effect.from))
            if(zhangyan->askForSkillInvoke(objectName(), data)){
                const Card *slash = room->askForCard(zhangyan, "slash", "@feiyan-slash:" + effect.from->objectName());
                if(slash){
                    room->playSkillEffect(objectName());
                    zhangyan->drawCards(1);

                    CardUseStruct use;
                    use.card = slash;
                    use.from = zhangyan;
                    use.to << effect.from;
                    room->useCard(use);
                }else{
                    LogMessage log;
                    log.type = "#FeiyanNoSlash";
                    log.from = zhangyan;
                    log.to << effect.from;
                    room->sendLog(log);
                }
            }

        return false;
    }
};

class Zhenhuo: public TriggerSkill{
public:
    Zhenhuo():TriggerSkill("zhenhuo"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *zhangbao, QVariant &data) const{
        Room *room = zhangbao->getRoom();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(!zhangbao->askForSkillInvoke(objectName(), data))
            return false;

        room->playSkillEffect(objectName());

        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(heart):(.*)");
        judge.good = false;
        judge.reason = objectName();
        judge.who = effect.to;

        room->judge(judge);

        if(judge.isBad()){
            QList<ServerPlayer *> players = room->getAlivePlayers(), enemies;
            foreach(ServerPlayer *player, players){
                if(effect.to->inMyAttackRange(player))
                    enemies << player;
            }

            ServerPlayer *enemy = room->askForPlayerChosen(zhangbao, enemies, objectName());

            DamageStruct damage;
            damage.card = NULL;
            damage.from = zhangbao;
            damage.to = enemy;
            damage.nature = DamageStruct::Fire;
            if(damage.from->hasSkill("jueqing")){
                LogMessage log;
                log.type = "#Jueqing";
                log.from = damage.from;
                log.to << damage.to;
                log.arg = QString::number(1);
                room->sendLog(log);
                room->playSkillEffect("jueqing");
                room->loseHp(damage.to, 1);
            }else{
                room->damage(damage);
            }
        }

        return false;
    }
};

class Heiyan: public ProhibitSkill{
public:
    Heiyan():ProhibitSkill("heiyan"){
        frequency = Compulsory;
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if(card->inherits("Duel") || card->inherits("Slash"))
            return card->isBlack();
        else
            return false;
    }
};

GuishuCard::GuishuCard(){
    target_fixed = true;
}

void GuishuCard::use(Room *room, ServerPlayer *zhangbao, const QList<ServerPlayer *> &targets) const{

}

class GuishuViewAsSkill:public OneCardViewAsSkill{
public:
    GuishuViewAsSkill():OneCardViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@guishu";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        GuishuCard *card = new GuishuCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Guishu: public TriggerSkill{
public:
    Guishu():TriggerSkill("guishu"){
        view_as_skill = new GuishuViewAsSkill;

        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(!TriggerSkill::triggerable(target))
            return false;

        if(target->isKongcheng()){
            bool has_red = false;
            int i;
            for(i=0; i<4; i++){
                const EquipCard *equip = target->getEquip(i);
                if(equip && equip->isRed()){
                    has_red = true;
                    break;
                }
            }

            return has_red;
        }else
            return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guishu-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        player->tag["Judge"] = data;
        const Card *card = room->askForCard(player, "@guishu", prompt);

        if(card){
            room->playSkillEffect(objectName());

            player->obtainCard(judge->card);

            judge->card = Sanguosha->getCard(card->getEffectiveId());
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }

        return false;
    }
};

WeichengCard::WeichengCard(){
    once = true;
    will_throw = false;
}

bool WeichengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty()
            && !to_select->isKongcheng()
            && to_select != Self
            && to_select->getHandcardNum() != Self->getHandcardNum();
}

void WeichengCard::use(Room *room, ServerPlayer *zhangmancheng, const QList<ServerPlayer *> &targets) const{
    bool success = zhangmancheng->pindian(targets.first(), "weicheng", this);

    if(success){
        QString choice = room->askForChoice(zhangmancheng, "weicheng", "skipdraw+skipplay");
        if(choice == "skipdraw"){
            targets.first()->setFlags("weicheng_skipdraw");

            LogMessage log;
            log.type = "#WeichengSkipdraw";
            log.from = zhangmancheng;
            log.to << targets.first();

            room->sendLog(log);
            room->playSkillEffect(objectName());
        }else if(choice == "skipplay"){
            targets.first()->setFlags("weicheng_skipplay");

            LogMessage log;
            log.type = "#WeichengSkipplay";
            log.from = zhangmancheng;
            log.to << targets.first();

            room->sendLog(log);
            room->playSkillEffect(objectName());
        }
    }else{
        zhangmancheng->setFlags("weicheng_failed");

        LogMessage log;
        log.type = "#WeichengFailed";
        log.from = zhangmancheng;
        log.to << targets.first();

        room->sendLog(log);
        room->playSkillEffect(objectName());
    }
}

class WeichengFailed: public TriggerSkill{
public:
    WeichengFailed():TriggerSkill("#weicheng"){
        events << CardUsed << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasFlag("weicheng_failed");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhangmancheng, QVariant &data) const{
        Room *room = zhangmancheng->getRoom();
        const Card *card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
            if(use.from == zhangmancheng)
                return false;
        }else if(event == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            card = effect.card;
            if(effect.from == zhangmancheng)
                return false;
        }

        if(card == NULL || !(card->inherits("Duel") || (card->inherits("Slash"))))
            return false;

        QList<int> equip_ids;
        QList<const Card *> equips = zhangmancheng->getEquips();
        foreach(const Card *c,equips)
            equip_ids << c->getId();

        room->fillAG(equip_ids, zhangmancheng);
        int card_id = room->askForAG(zhangmancheng, equip_ids, false, objectName());
        room->throwCard(card_id);

        LogMessage log;
        log.type = "#WeichengDiscard";
        log.from = zhangmancheng;

        room->sendLog(log);
        room->broadcastInvoke("clearAG");

        return false;
    }
};

class WeichengViewAsSkill: public OneCardViewAsSkill{
public:
    WeichengViewAsSkill():OneCardViewAsSkill("weicheng"){
        default_choice = "skipdraw";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("WeichengCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new WeichengCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Weicheng: public PhaseChangeSkill{
public:
    Weicheng():PhaseChangeSkill("weicheng"){
        view_as_skill = new WeichengViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasFlag("weicheng_skipdraw")
                || target->hasFlag("weicheng_skipplay")
                || target->hasFlag("weicheng_failed");
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();

        LogMessage log;
        log.type = "#WeichengClear";
        log.from = target;

        if(target->getPhase() == Player::Start && target->hasFlag("weicheng_failed"))
            room->setPlayerFlag(target, "-weicheng_failed");
        else if(target->getPhase() == Player::Draw && target->hasFlag("weicheng_skipdraw"))
            return true;
        else if(target->getPhase() == Player::Play && target->hasFlag("weicheng_skipplay"))
            return true;
        else if(target->getPhase() == Player::Finish){
            if(target->hasFlag("weicheng_skipdraw")){
                room->setPlayerFlag(target, "-weicheng_skipdraw");
                room->sendLog(log);
            }else if(target->hasFlag("weicheng_skipplay")){
                room->setPlayerFlag(target, "-weicheng_skipplay");
                room->sendLog(log);
            }
        }

        return false;
    }
};

class HuangjinBaonue: public TriggerSkill{
public:
    HuangjinBaonue():TriggerSkill("huangjin_baonue"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->isWounded()
           && damage.from->askForSkillInvoke(objectName(), data))
        {
            room->playSkillEffect(objectName());

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = damage.to;

            room->judge(judge);

            if(judge.isBad()){
                LogMessage log;
                log.type = "#HuangjinBaonueEffect";
                log.from = player;
                log.to << damage.to;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(damage.damage + 1);

                room->sendLog(log);

                damage.damage ++;
                data = QVariant::fromValue(damage);
            }
        }

        return false;
    }
};

class Jieliang: public TriggerSkill{
public:
    Jieliang():TriggerSkill("jieliang"){
        events << SlashHit;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        int card_num = 2 + qMin(player->getMark("@struggle"),3);
        if(player->hasFlag("luoyi"))
            card_num++;
        if(effect.drank)
            card_num++;

        if(!effect.to->isNude() && player->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            while(card_num > 0 && !effect.to->isNude()){
                int card_id = room->askForCardChosen(player, effect.to, "he", objectName());
                if(room->getCardPlace(card_id) == Player::Hand)
                    room->moveCardTo(Sanguosha->getCard(card_id), player, Player::Hand, false);
                else
                    room->obtainCard(player, card_id);
                card_num--;
            }
            return true;
        }

        return false;
    }
};

class Duoma: public PhaseChangeSkill{
public:
    Duoma():PhaseChangeSkill("duoma"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *peiyuanshao) const{
        Room *room = peiyuanshao->getRoom();
        if(peiyuanshao->getPhase() != Player::Draw)
            return false;

        QList<ServerPlayer *> players = room->getOtherPlayers(peiyuanshao);
        foreach(ServerPlayer *p, players){
            if(!p->hasEquip())
                players.removeOne(p);
        }

        if(players.isEmpty() || !peiyuanshao->askForSkillInvoke(objectName()))
            return false;

        ServerPlayer *target = room->askForPlayerChosen(peiyuanshao, players, objectName());
        QList<int> equip_ids;
        QList<const Card *> equips = target->getEquips();
        foreach(const Card *c,equips)
            equip_ids << c->getId();

        room->fillAG(equip_ids, peiyuanshao);
        int card_id = room->askForAG(peiyuanshao, equip_ids, false, objectName());;
        peiyuanshao->obtainCard(Sanguosha->getCard(card_id));
        peiyuanshao->drawCards(1);
        room->broadcastInvoke("clearAG");
        room->playSkillEffect(objectName());

        return true;
    }
};

class Pengdao: public TriggerSkill{
public:
    Pengdao():TriggerSkill("pengdao"){
        events << SlashMissed << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == SlashMissed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.to->hasSkill("kongcheng") && effect.to->isKongcheng())
                return false;

            const Card *card = room->askForCard(player, "slash", "@pengdao-slash");
            if(card){
                player->addMark("pengdaoslash");
                if(player->hasFlag("drank"))
                    room->setPlayerFlag(player, "-drank");

                CardUseStruct use;
                use.card = card;
                use.from = player;
                use.to << effect.to;
                room->useCard(use, false);
            }else{
                if(player->getMark("pengdaoslash") > 0){
                    LogMessage log;
                    log.type = "#PengdaoMiss";
                    log.from = player;
                    log.to << effect.to;

                    room->sendLog(log);
                    room->playSkillEffect(objectName());

                    player->setMark("pengdaoslash", 0);
                }
            }
        }else if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.card && damage.card->inherits("Slash") && player->getMark("pengdaoslash") > 0){
                Room *room = damage.to->getRoom();

                LogMessage log;
                log.type = "#PengdaoEffect";
                log.from = player;
                log.to << damage.to;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(damage.damage + player->getMark("pengdaoslash"));

                room->sendLog(log);
                room->playSkillEffect(objectName());

                damage.damage = damage.damage + player->getMark("pengdaoslash");
                data = QVariant::fromValue(damage);

                player->setMark("pengdaoslash", 0);
            }
        }
        return false;
    }
};

class Chixie: public PhaseChangeSkill{
public:
    Chixie():PhaseChangeSkill("chixie"){
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Discard && !player->getArmor()){
            if(player->getHandcardNum() > player->getHp())
                player->getRoom()->playSkillEffect(objectName());
        }
        return false;
    }
};

class Leichui: public TriggerSkill{
public:
    Leichui():TriggerSkill("leichui"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *mayuanyi, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()){
            Room *room = mayuanyi->getRoom();
            if(room->askForSkillInvoke(mayuanyi, objectName(), data)){
                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade|club):(.*)");
                judge.good = false;
                judge.reason = objectName();
                judge.who = damage.to;

                room->judge(judge);

                if(judge.isBad()){
                    DamageStruct damage;
                    damage.card = NULL;
                    damage.from = mayuanyi;
                    damage.to = judge.who;
                    damage.nature = DamageStruct::Thunder;

                    room->damage(damage);
                }
            }
        }

        return false;
    }
};

class Mabi: public TriggerSkill{
public:
    Mabi():TriggerSkill("mabi"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") && player->askForSkillInvoke(objectName()))
        {
            damage.to->setFlags("mabi");

            LogMessage log;
            log.type = "#MabiSkipdraw";
            log.from = player;
            log.to << damage.to;

            room->sendLog(log);
            room->playSkillEffect(objectName());

            return true;
        }

        return false;
    }
};

class MabiSkip: public PhaseChangeSkill{
public:
    MabiSkip():PhaseChangeSkill("#mabi"){
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();

        LogMessage log;
        log.type = "#MabiClear";
        log.from = target;

        if(target->getPhase() == Player::Draw && target->hasFlag("mabi"))
            return true;
        else if(target->getPhase() == Player::Finish && target->hasFlag("mabi")){
            room->setPlayerFlag(target, "-mabi");
            room->sendLog(log);
        }

        return false;
    }
};

class Xiuzhen: public TriggerSkill{
public:
    Xiuzhen():TriggerSkill("xiuzhen"){
        events << Damaged;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *xianyuji, QVariant &data) const{
        Room *room = xianyuji->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *source = damage.from;
        int nDamage = damage.damage;
        if(!source || !xianyuji->askForSkillInvoke(objectName()))
            return false;

        room->playSkillEffect(objectName());
        while(nDamage > 0){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(.*):(.*)");
            judge.good = false;
            judge.who = source;
            judge.reason = objectName();

            room->judge(judge);
            switch(judge.card->getSuit()){
                case Card::Heart:{
                        RecoverStruct recover;
                        recover.who = xianyuji;
                        room->recover(xianyuji, recover);
                        break;
                    }
                case Card::Diamond:{
                        xianyuji->drawCards(2);
                        break;
                    }
                case Card::Club:{
                        if(source && source->isAlive() && source->getHandcardNum() > 0)
                            room->askForDiscard(source, "xiuzhen", 2, false, true);
                        break;
                    }
                case Card::Spade:{
                        if(source && source->isAlive()){
                            DamageStruct damage;
                            damage.card = NULL;
                            damage.from = xianyuji;
                            damage.to = source;
                            damage.nature = DamageStruct::Thunder;

                            if(damage.from->hasSkill("jueqing")){
                                LogMessage log;
                                log.type = "#Jueqing";
                                log.from = damage.from;
                                log.to << damage.to;
                                log.arg = QString::number(1);
                                room->sendLog(log);
                                room->playSkillEffect("jueqing");
                                room->loseHp(damage.to, 1);
                            }else{
                                room->damage(damage);
                            }
                        }
                        break;
                    }
                default:
                    break;
            }

            nDamage--;
        }

        return false;
    }
};

class Huoqi: public TriggerSkill{
public:
    Huoqi():TriggerSkill("huoqi"){
        events << CardLost << PhaseChange;
        default_choice = "huoqirecover";
    }

    void perform(ServerPlayer *xiannanhua) const{
        Room *room = xiannanhua->getRoom();

        QList<ServerPlayer *> players, pindianer;
        foreach(ServerPlayer *p, room->getAlivePlayers())
            if(!p->isKongcheng())
                players << p;

        QString result;
        if(players.length() >= 2)
            result = room->askForChoice(xiannanhua, objectName(), "huoqirecover+huoqipindian+huoqinothing");
        else
            result = room->askForChoice(xiannanhua, objectName(), "huoqirecover+huoqinothing");

        if(result == "huoqirecover"){
            RecoverStruct recover;
            recover.who = xiannanhua;
            room->recover(xiannanhua, recover);
            room->playSkillEffect(objectName(), 1);
        }else if(result == "huoqipindian"){
            if(players.length()>=2){
                while(pindianer.length()<2){
                    ServerPlayer *p = room->askForPlayerChosen(xiannanhua, players, objectName());
                    pindianer << p;
                    players.removeOne(p);
                    room->getThread()->delay(500);
                }

                room->playSkillEffect(objectName(), 2);
                bool success = pindianer.first()->pindian(pindianer.last(), "huoqi", NULL);
                if(success){
                    if(!room->askForDiscard(pindianer.last(), objectName(), 2, true, true)){
                        DamageStruct damage;
                        damage.from = pindianer.first();
                        damage.to = pindianer.last();

                        if(damage.from->hasSkill("jueqing")){
                            LogMessage log;
                            log.type = "#Jueqing";
                            log.from = damage.from;
                            log.to << damage.to;
                            log.arg = QString::number(1);
                            room->sendLog(log);
                            room->playSkillEffect("jueqing");
                            room->loseHp(damage.to, 1);
                        }else{
                            room->damage(damage);
                        }
                    }
                }else{
                    if(!room->askForDiscard(pindianer.first(), objectName(), 2, true, true)){
                        DamageStruct damage;
                        damage.from = pindianer.last();
                        damage.to = pindianer.first();

                        if(damage.from->hasSkill("jueqing")){
                            LogMessage log;
                            log.type = "#Jueqing";
                            log.from = damage.from;
                            log.to << damage.to;
                            log.arg = QString::number(1);
                            room->sendLog(log);
                            room->playSkillEffect("jueqing");
                            room->loseHp(damage.to, 1);
                        }else{
                            room->damage(damage);
                        }
                    }
                }
            }
        }
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *xiannanhua, QVariant &data) const{
        if(xiannanhua->getPhase() == Player::Discard && event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->to_place == Player::DiscardedPile)
                xiannanhua->addMark("huoqi");
        }else if(xiannanhua->getPhase() == Player::Finish){
            if(xiannanhua->getMark("huoqi") >= 2){
                xiannanhua->setMark("huoqi", 0);
                if(xiannanhua->askForSkillInvoke(objectName()))
                    perform(xiannanhua);
            }else
                xiannanhua->setMark("huoqi", 0);
        }

        return false;
    }
};

class Yuli: public TriggerSkill{
public:
    Yuli():TriggerSkill("yuli"){
        events << Pindian;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *xiannanhua, QVariant &data) const{
        if(xiannanhua->askForSkillInvoke(objectName())){
            xiannanhua->getRoom()->playSkillEffect(objectName());
            xiannanhua->drawCards(1);
        }
        return false;
    }
};

TianbianCard::TianbianCard(){
    target_fixed = true;
    will_throw = false;
}

void TianbianCard::use(Room *room, ServerPlayer *xiannanhua, const QList<ServerPlayer *> &targets) const{
}

class TianbianViewAsSkill:public OneCardViewAsSkill{
public:
    TianbianViewAsSkill():OneCardViewAsSkill(""){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@tianbian";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        TianbianCard *card = new TianbianCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Tianbian: public TriggerSkill{
public:
    Tianbian():TriggerSkill("tianbian"){
        events << Pindian;
        view_as_skill = new TianbianViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player->hasSkill(objectName()) && !player->isKongcheng();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *xiannanhua, QVariant &data) const{
        Room *room = xiannanhua->getRoom();
        PindianStar pindian = data.value<PindianStar>();
        const Card *card = room->askForCard(xiannanhua, "@tianbian", "@tianbian-card");
        if(card){
            room->playSkillEffect(objectName());

            QList<ServerPlayer *> pindianer;
            pindianer << pindian->from;
            pindianer << pindian->to;

            ServerPlayer *p = room->askForPlayerChosen(xiannanhua, pindianer, objectName());

            if(p == pindian->from){
                room->moveCardTo(pindian->from_card, xiannanhua, Player::Hand, false);
                pindian->from_card = Sanguosha->getCard(card->getEffectiveId());
                room->moveCardTo(card, pindian->from, Player::Special, false);
            }else{
                room->moveCardTo(pindian->to_card, xiannanhua, Player::Hand, false);
                pindian->to_card = Sanguosha->getCard(card->getEffectiveId());
                room->moveCardTo(card, pindian->to, Player::Special, false);
            }

            data = QVariant::fromValue(pindian);

            LogMessage log;
            log.type = "$ChangedPindian";
            log.from = xiannanhua;
            log.to << p;

            room->sendLog(log);
        }
        return false;
    }
};

HuangjinPackage::HuangjinPackage()
    :Package("huangjin")
{
    General *zhangliang, *zhangyan, *zhangbao, *zhangmancheng, *chengyuanzhi, *guanhai, *peiyuanshao, *zhoucang,
    *mayuanyi, *xianyuji, *xiannanhua;

    zhangliang = new General(this, "zhangliang", "qun");
    zhangliang->addSkill(new Rengong);
    zhangliang->addSkill(new Shishi);

    zhangyan = new General(this, "zhangyan", "qun");
    zhangyan->addSkill(new Feiyan);

    zhangbao = new General(this, "zhangbao", "qun", 3);
    zhangbao->addSkill(new Zhenhuo);
    zhangbao->addSkill(new Heiyan);
    zhangbao->addSkill(new Guishu);

    zhangmancheng = new General(this, "zhangmancheng", "qun");
    zhangmancheng->addSkill(new Weicheng);
    zhangmancheng->addSkill(new WeichengFailed);

    related_skills.insertMulti("weicheng", "#weicheng");

    chengyuanzhi = new General(this, "chengyuanzhi", "qun");
    chengyuanzhi->addSkill(new HuangjinBaonue);

    guanhai = new General(this, "guanhai", "qun");
    guanhai->addSkill(new Jieliang);

    peiyuanshao = new General(this, "peiyuanshao", "qun");
    peiyuanshao->addSkill(new Duoma);

    zhoucang = new General(this, "zhoucang", "qun");
    zhoucang->addSkill(new Pengdao);
    zhoucang->addSkill(new Chixie);

    mayuanyi = new General(this, "mayuanyi", "qun");
    mayuanyi->addSkill(new Leichui);

    xianyuji = new General(this, "xianyuji", "god", 3);
    xianyuji->addSkill(new Mabi);
    xianyuji->addSkill(new MabiSkip);
    xianyuji->addSkill(new Xiuzhen);

    related_skills.insertMulti("mabi", "#mabi");

    xiannanhua = new General(this, "xiannanhua", "god", 3);
    xiannanhua->addSkill(new Huoqi);
    xiannanhua->addSkill(new Yuli);
    xiannanhua->addSkill(new Tianbian);

    addMetaObject<GuishuCard>();
    addMetaObject<WeichengCard>();
    addMetaObject<TianbianCard>();
}

ADD_PACKAGE(Huangjin)
