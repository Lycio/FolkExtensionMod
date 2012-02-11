#include "ghost.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "maneuvering.h"

class Sheji: public SlashBuffSkill{
public:
    Sheji():SlashBuffSkill("sheji"){
        frequency = Compulsory;
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *player = effect.from;
        Room *room = player->getRoom();
        if(player->getPhase() != Player::Play)
            return false;

        if(effect.to->inMyAttackRange(player)){
            room->playSkillEffect(objectName());
            room->slashResult(effect, NULL);

                return true;
        }

        return false;
    }
};

class Wumo: public TriggerSkill{
public:
    Wumo():TriggerSkill("wumo"){
        events << CardUsed << CardResponsed;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        const Card *card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            card = effect.card;
        }

        if(card == NULL)
            return false;

        if(card->inherits("Slash") && player->getPhase() == Player::Play){
            if(player->askForSkillInvoke(objectName(), data))
                player->drawCards(1);
        }

        return false;
    }
};

class Tuodao: public TriggerSkill{
public:
    Tuodao():TriggerSkill("tuodao"){
        events << SlashMissed;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }
    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(effect.to->hasSkill("tuodao") && effect.to->getPhase() == Player::NotActive)
            effect.to->getRoom()->askForUseCard(effect.to, "slash", "@askforslash");

        return false;
    }
};

class Xiaoshou:public MasochismSkill{
public:
    Xiaoshou():MasochismSkill("xiaoshou"){

    }

    virtual void onDamaged(ServerPlayer *guihuaxiong, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = guihuaxiong->getRoom();
        QVariant data = QVariant::fromValue(from);
        if(from && from->hasEquip() && room->askForSkillInvoke(guihuaxiong, "xiaoshou", data)){
            int card_id = room->askForCardChosen(guihuaxiong, from, "e", "xiaoshou");
            const Card *card = Sanguosha->getCard(card_id);
            room->obtainCard(guihuaxiong, card_id);

            QList<ServerPlayer *> targets = room->getAllPlayers();
            ServerPlayer *target = room->askForPlayerChosen(guihuaxiong, targets, "xiaoshou");
            if(target != guihuaxiong)
                room->moveCardTo(card, target, Player::Hand, true);
            QString choice = room->askForChoice(guihuaxiong, "xiaoshou", "obtain+equip");
            if(choice == "equip")
                room->moveCardTo(card, target, Player::Equip, true);
            room->playSkillEffect(objectName());
        }
    }
};


ShouyeGhostCard::ShouyeGhostCard(){
    once = true;
}

bool ShouyeGhostCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 1)
        return false;

    return true;
}

void ShouyeGhostCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(2);
}

class ShouyeGhost: public OneCardViewAsSkill{
public:
    ShouyeGhost():OneCardViewAsSkill("shouyeghost"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ShouyeGhostCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShouyeGhostCard *shouyeghost_card = new ShouyeGhostCard;
        shouyeghost_card->addSubcard(card_item->getCard()->getId());

        return shouyeghost_card;
    }
};



JiehuohCard::JiehuohCard(){
    target_fixed = true;
}

void JiehuohCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
}

class JiehuohViewAsSkill: public ViewAsSkill{
public:
    JiehuohViewAsSkill(): ViewAsSkill("jiehuoh"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasFlag("JieHuoCanUse") && player->getMark("@disabuse") >= 1;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 4)
            return false;

        if(to_select->isEquipped())
            return false;

        foreach(CardItem *item, selected){
            if(to_select->getFilteredCard()->getSuit() == item->getFilteredCard()->getSuit())
                return false;
        }

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 4)
            return NULL;

        JiehuohCard *card = new JiehuohCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        return card;
    }
};

class Jiehuoh: public TriggerSkill{
public:
    Jiehuoh():TriggerSkill("jiehuoh"){
        events  << Death << PhaseChange << CardUsed;
        frequency = Limited;
        view_as_skill = new JiehuohViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->findPlayerBySkillName(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *skillowner = room->findPlayerBySkillName(objectName());
        if(!skillowner)
            return false;

        static QStringList deadnames;
        if(event == Death){
            if(deadnames.isEmpty())
                room->setPlayerFlag(skillowner, "JieHuoCanUse");
            deadnames.append(player->getGeneralName());
            return false;
        }else if(event == PhaseChange && skillowner->getPhase() == Player::Finish){
            deadnames.clear();
            room->setPlayerFlag(skillowner, "-JieHuoCanUse");
        }else if(event == CardUsed && skillowner->getPhase() == Player::Play){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->getSkillName() != "jiehuoh")
                return false;
            QString choice = room->askForChoice(skillowner, objectName(), deadnames.join("+"));
            ServerPlayer *target = room->findPlayer(choice, true);
            room->revivePlayer(target);
            room->setPlayerProperty(target, "hp", 3);
            target->drawCards(3);
            player->loseMark("@disabuse");
            return false;
        }
        return false;
    }
};

class Qinwang: public TriggerSkill{
public:
    Qinwang():TriggerSkill("qinwang"){
        events << Predamaged << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->findPlayerBySkillName(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *shamoke = room->findPlayerBySkillName(objectName());

        if(event == Predamaged && shamoke->getPhase() == Player::NotActive && damage.to != shamoke){
            if(shamoke->askForSkillInvoke(objectName())){
                damage.to = shamoke;
                damage.chain = true;
                room->damage(damage);
                shamoke->gainMark("@dutiful", damage.damage);

                return true;
            }

        }
        else if(event == Predamaged && shamoke->getPhase() == Player::Play && damage.from == shamoke){
            const Card *card = damage.card;
            if(!(card && (card->inherits("Slash") || card->inherits("Duel"))))
                return false;
            int x = shamoke->getMark("@dutiful");
            if(x > 0){
                int y = damage.damage;
                damage.damage += x;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#QinwangBuff";
                log.from = shamoke;
                log.arg = QString::number(y);
                log.arg2 = QString::number(damage.damage);
                shamoke->getRoom()->sendLog(log);
            }
        }
        else if(event == PhaseChange && player == shamoke && player->getPhase() == Player::Finish){
            int x = player->getMark("@dutiful");
            if(x > 0){
                player->loseMark("@dutiful", x);
            }
        }
        return false;
    }
};

class Zhuangshen:public TriggerSkill{
public:
    Zhuangshen():TriggerSkill("zhuangshen"){
        events << PhaseChange << FinishJudge;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *guizhuge, QVariant &data) const{
        if(event == PhaseChange && guizhuge->getPhase() == Player::Start){
            Room *room = guizhuge->getRoom();
            static QStringList skill_names;
            foreach(QString skillstr, skill_names){
                room->detachSkillFromPlayer(guizhuge, skillstr);
            }
            skill_names.clear();

            if(guizhuge->askForSkillInvoke("zhuangshen")){
                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade|club):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = guizhuge;

                room->judge(judge);
                if(judge.isBad())
                    return false;

                QList<ServerPlayer *> players = room->getOtherPlayers(guizhuge);
                ServerPlayer *target = room->askForPlayerChosen(guizhuge, players, objectName());
                QList<const Skill *> skills = target->getVisibleSkillList();

                foreach(const Skill *skill, skills){
                    if(skill->isLordSkill() || skill->getFrequency() == Skill::Limited
                       || skill->getFrequency() == Skill::Wake)
                        continue;

                    skill_names << skill->objectName();
                }
                if(!skill_names.isEmpty()){
                    foreach(QString skillstr, skill_names){
                        room->acquireSkill(guizhuge, skillstr);
                    }
                }
            }

        }

        return false;
    }
};

class Qimen: public ProhibitSkill{
public:
    Qimen():ProhibitSkill("qimen"){

    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("DelayedTrick");
    }
};

GhostPackage::GhostPackage()
    :Package("ghost")
{
    General *guizhangfei = new General(this, "guizhangfei", "shu", 4);
    guizhangfei->addSkill(new Skill("longyin", Skill::Compulsory));
    guizhangfei->addSkill(new Skill("huxiao", Skill::Compulsory));

    General *guilvbu = new General(this, "guilvbu", "qun", 4);
    guilvbu->addSkill(new Sheji);
    guilvbu->addSkill(new Skill("juelu", Skill::Compulsory));

    General *guiguanyu = new General(this, "guiguanyu", "shu", 4);
    guiguanyu->addSkill(new Wumo);
    guiguanyu->addSkill(new Tuodao);

    General *guihuaxiong = new General(this, "guihuaxiong", "qun", 4);
    guihuaxiong->addSkill(new Xiaoshou);

    General *guisimahui = new General(this, "guisimahui", "qun", 3);
    guisimahui->addSkill(new ShouyeGhost);
    guisimahui->addSkill(new MarkAssignSkill("@disabuse", 1));
    guisimahui->addSkill(new Jiehuoh);

    General *shamoke = new General(this, "shamoke", "shu", 4);
    shamoke->addSkill(new Qinwang);

    General *guizhuge = new General(this, "guizhuge", "shu", 3);
    guizhuge->addSkill(new Zhuangshen);
    guizhuge->addSkill(new Qimen);

    addMetaObject<ShouyeGhostCard>();
    addMetaObject<JiehuohCard>();
}

ADD_PACKAGE(Ghost)
