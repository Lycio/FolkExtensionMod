#include "Yan-package.h"
#include "skill.h"
#include "standard.h"
#include "maneuvering.h"
#include "dishacardpackage.h"
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

class YanYanhu: public OneCardViewAsSkill{
public:
    YanYanhu():OneCardViewAsSkill("yanyanhu"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "cover";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        Cover *acard = new Cover(card->getSuit(), card->getNumber());
        acard->setSkillName(objectName());
        acard->addSubcard(card);
        return acard;
    }
};

class YanFanji: public TriggerSkill{
public:
    YanFanji():TriggerSkill("yanfanji"){
        events << CardFinished;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->inherits("Cover"))
            return false;
        if(!player->askForSkillInvoke(objectName(), data))
            return false;
        ServerPlayer *source = room->getTag("CoverSource").value<PlayerStar>();
        bool win = player->pindian(source, objectName(), NULL);
        if(win){
            if(!player->isWounded())
                player->drawCards(2);
            else{
                if(room->askForChoice(player, objectName(), "recoverhp+drawcards") == "recoverhp"){
                    RecoverStruct recover;
                    recover.card = NULL;
                    recover.who = player;
                    recover.recover = 1;
                    room->recover(player, recover);
                }else
                    player->drawCards(2);
            }
        }
        return false;
    }
};

class YanShixue: public FilterSkill{
public:
    YanShixue():FilterSkill("yanshixue"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        BloodSlash *b_slash = new BloodSlash(c->getSuit(), c->getNumber());
        b_slash->setSkillName(objectName());
        b_slash->addSubcard(card_item->getCard());

        return b_slash;
    }
};

class YanXianfeng: public OneCardViewAsSkill{
public:
    YanXianfeng():OneCardViewAsSkill("yanxianfeng"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "sudden_strike";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        SuddenStrike *acard = new SuddenStrike(card->getSuit(), card->getNumber());
        acard->setSkillName(objectName());
        acard->addSubcard(card);
        return acard;
    }
};

class YanJiaozhan: public TriggerSkill{
public:
    YanJiaozhan():TriggerSkill("yanjiaozhan"){
        events << PhaseChange << Damage;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *yanhuaxiong = room->findPlayerBySkillName(objectName());
        if(!yanhuaxiong)
            return false;

        if(event == PhaseChange){
            if(yanhuaxiong->getPhase() == Player::Finish){
                if(!yanhuaxiong->askForSkillInvoke(objectName()))
                    return false;
                ServerPlayer *target = room->askForPlayerChosen(yanhuaxiong, room->getOtherPlayers(yanhuaxiong), objectName());
                room->setTag("JiaozhanTarget", QVariant::fromValue(target));
                target->gainMark("@jiaozhan", 1);
            }else if(player->getPhase() == Player::Play){
                if(player->getMark("@jiaozhan") == 1){
                    player->loseMark("@jiaozhan", 1);
                    Duel *duel = new Duel(Card::NoSuit, 0);
                    duel->setSkillName(objectName());
                    duel->setCancelable(false);
                    CardUseStruct use;
                    use.card = duel;
                    use.from = yanhuaxiong;
                    use.to << player;
                    room->useCard(use);
                }
            }
        }else if(event == Damage){
            if(room->getTag("JiaozhanTarget").isNull())
                return false;
            PlayerStar target = room->getTag("JiaozhanTarget").value<PlayerStar>();
            room->removeTag("JiaozhanTarget");
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.card->getSkillName() == objectName() && damage.to->objectName() == target->objectName())
                yanhuaxiong->drawCards(2);
        }
        return false;
    }
};

YanJiushaCard::YanJiushaCard(){
    target_fixed = true;
}

void YanJiushaCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    const Card *card = Sanguosha->getCard(this->getSubcards().first());
    room->throwCard(this);
    ServerPlayer *target = source;
    foreach(ServerPlayer *p, room->getAllPlayers()){
        if(p->hasFlag("dying")){
            target = p;
            break;
        }
    }
    CardUseStruct use;
    use.from = source;
    use.to << target;
    Card *cd = NULL;
    if(target->objectName() == source->objectName()){
        if(source->isWounded()){
            if(room->askForChoice(source, "yanjiusha", "peach+poison_peach") == "peach")
                cd = new Peach(card->getSuit(), card->getNumber());
            else
                cd = new PoisonPeach(card->getSuit(), card->getNumber());
        }else
            cd = new PoisonPeach(card->getSuit(), card->getNumber());
    }else{
        if(room->askForChoice(source, "yanjiusha", "peach+poison_peach") == "peach")
            cd = new Peach(card->getSuit(), card->getNumber());
        else
            cd = new PoisonPeach(card->getSuit(), card->getNumber());
    }
    cd->setSkillName("yanjiusha");
    use.card = cd;
    room->useCard(use);
}

class YanJiusha: public OneCardViewAsSkill{
public:
    YanJiusha():OneCardViewAsSkill("yanjiusha"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Heart && to_select->getFilteredCard()->inherits("Jink");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return true;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("peach");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        Card *acard = new YanJiushaCard;
        acard->addSubcard(card);
        return acard;
    }
};

class YanShenyi: public TriggerSkill{
public:
    YanShenyi():TriggerSkill("yanshenyi"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->inherits("Peach") || player->getPhase() != Player::NotActive)
            return false;
        if(!player->askForSkillInvoke(objectName(), data))
            return false;

        RecoverStruct recover;
        recover.card = NULL;
        recover.who = player;
        recover.recover = 1;
        room->recover(player, recover);

        return false;
    }
};

class YanJiefu: public OneCardViewAsSkill{
public:
    YanJiefu():OneCardViewAsSkill("yanjiefu"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->isBlack();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "rob";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        Rob *acard = new Rob(card->getSuit(), card->getNumber());
        acard->setSkillName(objectName());
        acard->addSubcard(card);
        return acard;
    }
};

YanJiuseCard::YanJiuseCard(){

}

bool YanJiuseCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() == 0)
        return to_select->getGeneral()->isMale();
    else if(targets.length() == 1)
        return targets.first()->canSlash(to_select);
    else if(targets.length() >= 2)
        return false;
    return false;
}

bool YanJiuseCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void YanJiuseCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    ServerPlayer *from = targets.at(0);
    ServerPlayer *to = targets.at(1);
    const Card *slash = Sanguosha->getCard(this->getSubcards().first());
    from->setFlags("jiuse");
    room->cardEffect(slash, from, to);
}

class YanJiuseViewAsSkill: public OneCardViewAsSkill{
public:
    YanJiuseViewAsSkill():OneCardViewAsSkill("yanjiuse"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Slash");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YanJiuseCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new YanJiuseCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class YanJiuse: public TriggerSkill{
public:
    YanJiuse():TriggerSkill("yanjiuse"){
        events << Predamage << CardFinished;
        view_as_skill = new YanJiuseViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            if(!damage.from->hasFlag("jiuse"))
                return false;
            damage.from->setFlags("-jiuse");
            damage.damage = 2;
            data = QVariant::fromValue(damage);
        }else if(event == CardFinished){
            foreach(ServerPlayer *p, player->getRoom()->getAllPlayers()){
                if(p->hasFlag("jiuse"))
                    p->setFlags("-jiuse");
            }
        }
        return false;
    }
};

class YanManwu: public TriggerSkill{
public:
    YanManwu():TriggerSkill("yanmanwu"){
        events << CardFinished << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *shendiaochan = room->findPlayerBySkillName(objectName());
        if(!shendiaochan)
            return false;
        static QList<int> manwulist;

        if(event == CardFinished){
            ServerPlayer *current = room->getCurrent();
            CardUseStruct use = data.value<CardUseStruct>();
            if(!use.from->getGeneral()->isMale() ||
                    current->hasFlag("manwu") ||
                    current->objectName() == shendiaochan->objectName() ||
                    room->getCardPlace(use.card->getEffectiveId()) != Player::DiscardedPile)
                return false;
            if(!shendiaochan->askForSkillInvoke(objectName(), data))
                return false;
            if(!use.card->isVirtualCard())
                shendiaochan->addToPile("yanmanwu", use.card->getEffectiveId(), true);
            else{
                QList<int> ids = use.card->getSubcards();
                room->fillAG(ids, shendiaochan);
                int id = room->askForAG(shendiaochan, ids, false, objectName());
                shendiaochan->invoke("clearAG");
                shendiaochan->addToPile("yanmanwu", id, true);
            }
            current->setFlags("manwu");
        }else if(event == PhaseChange){
            if(shendiaochan->getPhase() == Player::Start){
                manwulist = shendiaochan->getPile("yanmanwu");
                foreach(int id, manwulist){
                    const Card *cd = Sanguosha->getCard(id);
                    room->moveCardTo(cd, shendiaochan, Player::Hand, true);
                }
            }else if(player->getPhase() == Player::Finish){
                if(player->objectName() == shendiaochan->objectName()){
                    while(!manwulist.isEmpty()){
                        int id = manwulist.takeFirst();
                        if(room->getCardPlace(id) == Player::Hand)
                            room->throwCard(id);
                    }
                }else{
                    if(player->hasFlag("manwu"))
                        player->setFlags("-manwu");
                }
            }
        }
        return false;
    }
};
/*
class YanMeihuo: public TriggerSkill{
public:
    YanMeihuo():TriggerSkill("yanmeihuo"){
        events << SlashEffect << CardFinished;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->findPlayerBySkillName(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *shendiaochan = room->findPlayerBySkillName(objectName());
        if(!shendiaochan)
            return false;

        if(event == CardFinished){
            CardUseStruct use = data.value<CardUseStruct>();
            if(!use.from->hasFlag("meihuo"))
                return false;
            use.from->setFlags("-meihuo");
            use.from->getGeneral()->setGender(General::Male);
        }else if(event == SlashEffect){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.to->objectName() == shendiaochan->objectName() && effect.from->getGeneral()->isMale()){
                effect.to->getGeneral()->setGender(General::Female);
                effect.to->setFlags("meihuo");
            }
        }
        return false;
    }
};
*/

class YanShenyou: public TriggerSkill{
public:
    YanShenyou():TriggerSkill("yanshenyou"){
        events << Predamage << Predamaged;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *shenlubu = room->findPlayer("shenlubu");
        if(!shenlubu)
            return false;

        DamageStruct damage = data.value<DamageStruct>();
        if(event == Predamage){
            if(shenlubu->getMark("@wrath") <= 0)
                return false;
            shenlubu->loseMark("@wrath", qMin(shenlubu->getMark("@wrath"), damage.damage));
        }else if(event == Predamaged){
            shenlubu->gainMark("@wrath", damage.damage);
        }
        return false;
    }
};

class YanDanqi: public DistanceSkill{
public:
    YanDanqi():DistanceSkill("yandanqi")
    {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(from->hasSkill(objectName()) && from->getWeapon())
            return -1;
        else if(to->hasSkill(objectName()) && to->getArmor())
            return 1;
        return 0;
    }
};

class YanYizhi: public TriggerSkill{
public:
    YanYizhi():TriggerSkill("yanyizhi"){
        events << CardEffected ;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(!player->getEquips().isEmpty())
            return false;
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.from && effect.from->objectName() != player->objectName() && effect.card->isNDTrick()){
            LogMessage log;
            log.type = "$YanYizhiLog";
            log.from = player;
            log.card_str = effect.card->toString();
            room->sendLog(log);
            return true;
        }
        return false;
    }
};

class YanJifeng: public TriggerSkill{
public:
    YanJifeng():TriggerSkill("yanjifeng"){
        events << PhaseChange ;
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName()) && target->getMark("yanjifeng") == 0;
    }

    void exchangeBloodCard(ServerPlayer *player, ServerPlayer *target) const{
        Room *room = player->getRoom();
        int to_max = target->getMaxHP(), to_hp = target->getHp(), from_max = player->getMaxHP(), from_hp = player->getHp();
        room->setPlayerProperty(player, "maxhp", QVariant(to_max));
        room->setPlayerProperty(player, "hp", QVariant(to_hp));
        room->setPlayerProperty(target, "maxhp", QVariant(from_max));
        room->setPlayerProperty(target, "hp", QVariant(from_hp));
        LogMessage log;
        log.type = "#ExchangeBloodCard";
        log.from = player;
        log.to << target;
        room->sendLog(log);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(player->getPhase() != Player::Start || !player->isKongcheng())
            return false;
        room->detachSkillFromPlayer(player, "yantianhui");
        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
        exchangeBloodCard(player, target);
        player->addMark("yanjifeng");
        return false;
    }
};

class YanTianhui: public OneCardViewAsSkill{
public:
    YanTianhui():OneCardViewAsSkill("yantianhui"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.contains("peach") && player->getPhase() == Player::NotActive;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        if(card->isBlack()){
            Analeptic *analeptic = new Analeptic(card->getSuit(), card->getNumber());
            analeptic->addSubcard(card);
            analeptic->setSkillName(objectName());
            return analeptic;
        }else if(card->isRed()){
            Peach *peach = new Peach(card->getSuit(), card->getNumber());
            peach->addSubcard(card);
            peach->setSkillName(objectName());
            return peach;
        }else
            return NULL;
    }
};

YanCangshanCard::YanCangshanCard(){
    target_fixed = true;
}

void YanCangshanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = source;
    foreach(ServerPlayer *p, room->getAllPlayers()){
        if(p->hasFlag("dying")){
            target = p;
            break;
        }
    }
    const Card *card = Sanguosha->getCard(source->getPile("shanpile").first());
    CardUseStruct use;
    use.from = source;
    Card *cd = NULL;
    if(target->objectName() == source->objectName()){
        if(source->isWounded()){
            if(room->askForChoice(source, "yancangshan", "peach+god_salvation") == "peach")
                cd = new Peach(card->getSuit(), card->getNumber());
            else
                cd = new GodSalvation(card->getSuit(), card->getNumber());
        }else
            cd = new GodSalvation(card->getSuit(), card->getNumber());
    }else{
        if(room->askForChoice(source, "yanjiusha", "peach+poison_peach") == "peach")
            cd = new Peach(card->getSuit(), card->getNumber());
        else
            cd = new GodSalvation(card->getSuit(), card->getNumber());
    }
    source->loseMark("@shan", 1);
    //room->throwCard(card);
    if(cd->inherits("Peach"))
        use.to << target;
    else
        use.to << room->getAllPlayers();
    cd->setSkillName("yancangshan");
    cd->addSubcard(card);
    use.card = cd;
    room->useCard(use);
}

class YanCangshanViewAsSkill: public ZeroCardViewAsSkill{
public:
    YanCangshanViewAsSkill():ZeroCardViewAsSkill("yancangshan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@shan") > 0;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("peach") && player->getMark("@shan") > 0;
    }

    virtual const Card *viewAs() const{
        return new YanCangshanCard;
    }
};

class YanCangshan: public TriggerSkill{
public:
    YanCangshan():TriggerSkill("yancangshan"){
        events << GameStart << CardUsed;
        view_as_skill = new YanCangshanViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == GameStart){
            player->drawCards(3);
            QList<int> handcards = player->handCards().mid(0, 7);
            while(handcards.length() > 4){
                room->fillAG(handcards, player);
                int id = room->askForAG(player, handcards, false, objectName());
                handcards.removeOne(id);
                player->addToPile("shanpile", id, false);
                player->invoke("clearAG");
            }
            player->gainMark("@shan", 3);
        }else if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->getSkillName() == objectName())
                player->loseMark("@shan", 1);
        }
        return false;
    }
};

class YanQunwu: public TriggerSkill{
public:
    YanQunwu():TriggerSkill("yanqunwu"){
        events << CardEffected ;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.multiple && effect.card->inherits("TrickCard")){
            LogMessage log;
            log.type = "#YanQunwuLog";
            log.from = player;
            log.arg = effect.card->objectName();
            room->sendLog(log);
            return true;
        }
        return false;
    }
};

class YanHuimouViewAsSkill: public ViewAsSkill{
public:
    YanHuimouViewAsSkill():ViewAsSkill("yanhuimou"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@yanhuimou";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        DummyCard *card = new DummyCard;
        card->setSkillName(objectName());
        card->addSubcards(cards);
        return card;
    }
};

class YanHuimou: public TriggerSkill{
public:
    YanHuimou():TriggerSkill("yanhuimou"){
        view_as_skill = new YanHuimouViewAsSkill;

        events << CardEffected ;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *shenxiaoqiao = room->findPlayerBySkillName(objectName());
        if(!shenxiaoqiao)
            return false;

        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.multiple && effect.card->inherits("TrickCard") && !shenxiaoqiao->isKongcheng()){
            CardStar card = room->askForCard(shenxiaoqiao, "@yanhuimou", QString("@yanhuimou:%1").arg(player->getGeneralName()));
            if(card){
                QList<int> card_ids = card->getSubcards();
                foreach(int card_id, card_ids){
                    LogMessage log;
                    log.type = "$DiscardCard";
                    log.from = shenxiaoqiao;
                    log.card_str = QString::number(card_id);

                    room->sendLog(log);
                }

                LogMessage log;
                log.type = "#YanHuimouLog";
                log.from = shenxiaoqiao;
                log.to << player;
                log.arg = effect.card->objectName();
                room->sendLog(log);
                return true;
            }
        }
        return false;
    }
};

class YanFenshang: public TriggerSkill{
public:
    YanFenshang():TriggerSkill("yanfenshang"){
        events << DrawNCards << Damaged ;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *shenxiaoqiao = room->findPlayerBySkillName(objectName());
        if(!shenxiaoqiao)
            return false;

        if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(!shenxiaoqiao->isAlive() || damage.to->objectName() != shenxiaoqiao->objectName())
                return false;
            int i;
            for(i=0; i<damage.damage; i++){
                if(!shenxiaoqiao->askForSkillInvoke(objectName(), data))
                    break;
                QList<ServerPlayer *> tos;
                foreach(ServerPlayer *p, room->getAllPlayers()){
                    if(p->getMark("@fenshang") == 0)
                        tos << p;
                }

                ServerPlayer *to = room->askForPlayerChosen(shenxiaoqiao, tos, objectName());
                to->gainMark("@fenshang", 1);
            }
        }else if(event == DrawNCards){
            if(player->getMark("@fenshang") > 0){
                int n = data.toInt();
                data = QVariant::fromValue(n + 2*player->getMark("@fenshang"));
                player->loseMark("@fenshang", player->getMark("@fenshang"));
            }
        }
        return false;
    }
};

class YanJiuzi: public GameStartSkill{
public:
    YanJiuzi():GameStartSkill("yanjiuzi"){

    }

    virtual int getPriority() const{
        return -1;
    }

    virtual void onGameStart(ServerPlayer *player) const{
        Card *adou = NULL;
        foreach(Card *card, Sanguosha->getCards()){
            if(card->inherits("AdouMark")){
                adou = card;
                break;
            }
        }
        player->getRoom()->moveCardTo(adou, player, Player::Equip, true);
    }
};

YanTuoguCard::YanTuoguCard(){

}

bool YanTuoguCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return !to_select->getArmor() && targets.isEmpty();
}

void YanTuoguCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *from = source, *to = targets.first();
    foreach(ServerPlayer *p, room->getOtherPlayers(source)){
        if(p->getArmor() && p->getArmor()->objectName() == "adou_mark"){
            from = p;
            break;
        }
    }
    const Card *adou = from->getArmor();
    room->moveCardTo(adou, to, Player::Equip, true);
}

class YanTuoguViewAsSkill: public ZeroCardViewAsSkill{
public:
    YanTuoguViewAsSkill():ZeroCardViewAsSkill("yantuogu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(Self->getArmor() && Self->getArmor()->objectName() == "adou_mark")
            return !player->hasUsed("YanTuoguCard");
        else{
            bool can_invoke = false;
            foreach(const Player *p, player->getSiblings()){
                if(p->getArmor() && p->getArmor()->objectName() == "adou_mark"){
                    can_invoke = true;
                    break;
                }
            }
            return can_invoke && !player->hasUsed("YanTuoguCard");
        }
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }

    virtual const Card *viewAs() const{
        return new YanTuoguCard;
    }
};

class YanTuogu: public TriggerSkill{
public:
    YanTuogu():TriggerSkill("yantuogu"){
        events << CardGot << CardLost ;
        view_as_skill = new YanTuoguViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        CardMoveStar move = data.value<CardMoveStar>();
        const Card *card = Sanguosha->getCard(move->card_id);
        if(!card->inherits("AdouMark"))
            return false;
        if(event == CardGot && move->to_place == Player::Equip){
            if(player->hasSkill("longdan"))
                room->detachSkillFromPlayer(player, "longdan");
        }else if(event == CardLost && move->from_place == Player::Equip){
            room->acquireSkill(player, "longdan");
        }
        return false;
    }
};

class YanLongtai: public TriggerSkill{
public:
    YanLongtai():TriggerSkill("yanlongtai"){
        events << CardEffected ;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *mifuren = room->findPlayerBySkillName(objectName());
        if(!mifuren)
            return false;
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.card->isNDTrick())
            return false;
        if(effect.to->getArmor() && effect.to->getArmor()->objectName() == "adou_mark"){
            const Card *card = room->askForCard(mifuren, ".|.|.|.|red",
                                                QString("@yanlongtai:%1::%2").arg(effect.to->getGeneralName()).arg(effect.card->objectName()), data);
            if(card){
                LogMessage log;
                log.type = "#YanLongtaiLog";
                log.from = mifuren;
                log.to << effect.to;
                log.arg = effect.card->objectName();
                log.arg2 = objectName();
                room->sendLog(log);
                return true;
            }
        }
        return false;
    }
};

class YanToujing: public TriggerSkill{
public:
    YanToujing():TriggerSkill("yantoujing"){
        events << CardLost ;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *mifuren = room->findPlayerBySkillName(objectName());
        if(!mifuren)
            return false;
        CardMoveStar move = data.value<CardMoveStar>();
        const Card *card = Sanguosha->getCard(move->card_id);
        if(card->inherits("AdouMark") && move->to_place == Player::DiscardedPile)
            room->loseHp(mifuren, mifuren->getHp());
        return false;
    }
};

class YanToujingDeath: public TriggerSkill{
public:
    YanToujingDeath():TriggerSkill("#yantoujingdeath"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), "yantoujing");
        LogMessage log;
        log.type = "#YanToujingLog";
        log.from = player;
        log.to << target;
        log.arg = "yantoujing";
        room->sendLog(log);
        room->acquireSkill(target, "longdan");
        return false;
    }
};

class YanHuixuan: public TriggerSkill{
public:
    YanHuixuan():TriggerSkill("yanhuixuan"){
        events << CardResponsed;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(card_star->inherits("Slash"))
            player->setFlags(objectName());

        return false;
    }
};

class YanHuixuanSkip: public PhaseChangeSkill{
public:
    YanHuixuanSkip():PhaseChangeSkill("#yanhuixuan-skip"){
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Start){
            player->setFlags("-yanhuixuan");
            if(player->getMark("@huixuan") > 0)
                player->loseMark("@huixuan", 1);
        }else if(player->getPhase() == Player::Discard){
            if(!player->hasFlag("yanhuixuan") &&
                    player->getSlashCount() == 0 &&
                    player->askForSkillInvoke("yanhuixuan") &&
                    player->getRoom()->askForDiscard(player, "yanhuixuan", 1, false, true)){
                player->getRoom()->acquireSkill(player, "yanhuixuan-buff");
                player->gainMark("@huixuan", 1);
                return true;
            }
        }
        return false;
    }
};

class YanHuixuanBuff: public TriggerSkill{
public:
    YanHuixuanBuff():TriggerSkill("yanhuixuan-buff"){
        events << SlashMissed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *lengyanju = room->findPlayerBySkillName(objectName());
        if(!lengyanju || lengyanju->getMark("@huixuan") == 0)
            return false;
        SlashEffectStruct ses = data.value<SlashEffectStruct>();
        const Card *card = room->askForCard(lengyanju, ".|.|.|.|red", "@yanhuixuan:"+ses.to->objectName(), data);
        if(card){
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card);
            slash->setSkillName(objectName());

            LogMessage log;
            log.type = "#YanHuixuanLog";
            log.from = lengyanju;
            log.to << ses.from;
            log.arg = ses.to->getGeneralName();
            log.arg2 = "yanhuixuan";
            room->sendLog(log);

            CardUseStruct use;
            use.card = slash;
            use.from = ses.from;
            use.to << ses.to;
            room->useCard(use, false);
        }

        return false;
    }
};

class YanDaohun: public TriggerSkill{
public:
    YanDaohun():TriggerSkill("yandaohun"){
        events << CardLost << FinishJudge;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return ! target->hasSkill(objectName());
    }

    const Card *getBlade(const Card *card) const{
        const Card *blade;

        foreach(int card_id, card->getSubcards()){
            const Card *c = Sanguosha->getCard(card_id);
            if(c->inherits("Blade"))
                blade = c;
        }

        return blade;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *lengyanju = room->findPlayerBySkillName(objectName());
        if(!lengyanju)
            return false;
        const Card *blade = NULL;

        if(event == CardLost){
            CardMoveStar cms = data.value<CardMoveStar>();
            if(cms->to_place == Player::DiscardedPile && cms->from->objectName() != lengyanju->objectName()){
                const Card *card = Sanguosha->getCard(cms->card_id);
                if(card->inherits("Blade"))
                    blade = card;
            }
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(room->getCardPlace(judge->card->getEffectiveId()) == Player::DiscardedPile
               && judge->card->inherits("Blade"))
               blade = judge->card;
        }
        if(blade == NULL)
            return false;

        if(lengyanju->askForSkillInvoke(objectName(), data)){
            QList<ServerPlayer *> tos;
            ServerPlayer *to = lengyanju;
            foreach(ServerPlayer *p, room->getAllPlayers())
                if(!p->getWeapon())
                    tos << p;
            if(tos.isEmpty())
                lengyanju->obtainCard(blade);
            else{
                to = room->askForPlayerChosen(lengyanju, tos, objectName());
                room->moveCardTo(blade, to, Player::Equip, true);
            }
        }

        return false;
    }
};

class YanLongyin: public TriggerSkill{
public:
    YanLongyin():TriggerSkill("yanlongyin"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player->getGeneralName().contains("guanyu") ||
                (player->getWeapon() && player->getWeapon()->objectName() == "blade");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *lengyanju = room->findPlayerBySkillName(objectName());
        if(!lengyanju)
            return false;
        DamageStruct ds = data.value<DamageStruct>();
        if(ds.card && ds.card->inherits("Slash") && ds.card->isRed() &&
                lengyanju->askForSkillInvoke(objectName(), data)){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(.*):(.*)");
            judge.who = lengyanju;
            judge.reason = objectName();
            room->judge(judge);

            if(judge.card->getSuit() == Card::Heart){
                ds.damage ++;
                data = QVariant::fromValue(ds);
            }
            else
                lengyanju->obtainCard(judge.card);
        }

        return false;
    }
};

class YanGuiling: public TriggerSkill{
public:
    YanGuiling():TriggerSkill("yanguiling"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStar ds = data.value<DamageStar>();
        if(!ds->from || ds->from->isDead())
            return false;
        LogMessage log;
        log.type = "#YanGuilingLog";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);
        room->acquireSkill(ds->from, "yanguiling-buff");
        ds->from->gainMark("@guiling", 1);
        if(ds->from->getWeapon())
            room->throwCard(ds->from->getWeapon());

        return false;
    }
};

class YanGuilingBuff: public TriggerSkill{
public:
    YanGuilingBuff():TriggerSkill("yanguiling-buff"){
        events << SlashMissed << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player->getMark("@guiling") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == SlashMissed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if((effect.to->hasSkill("kongcheng") && effect.to->isKongcheng()))
                return false;

            const Card *card = room->askForCard(player, "slash", "@guiling-slash:" + effect.to->objectName());
            if(card){
                if(player->hasFlag("drank"))
                    room->setPlayerFlag(player, "-drank");

                CardUseStruct use;
                use.card = card;
                use.from = player;
                use.to << effect.to;
                room->useCard(use, false);
            }
        }else if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->inherits("Weapon")){
                room->throwCard(use.card);
                return true;
            }
        }
        return false;
    }
};
/*
class YanShenbing: public MasochismSkill{
public:
    YanShenbing():MasochismSkill("yanshenbing"){
        frequency = Frequent;
    }





    virtual void onDamaged(ServerPlayer *shenluxun, const DamageStruct &damage) const{
        int n = damage.damage;
        if(n == 0)
            return;

        if(shenluxun->askForSkillInvoke(objectName())){
            Huashen::AcquireGenerals(shenluxun, n);
        }
    }



class Xinsheng: public MasochismSkill{
public:
    Xinsheng():MasochismSkill("xinsheng"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *shenluxun, const DamageStruct &damage) const{
        int n = damage.damage;
        if(n == 0)
            return;

        if(shenluxun->askForSkillInvoke(objectName())){
            Huashen::PlayEffect(shenluxun, objectName());
            Huashen::AcquireGenerals(shenluxun, n);
        }
    }
};
*/

class YanShenbing: public MasochismSkill{
public:
    YanShenbing():MasochismSkill("yanshenbing"){
        frequency = Frequent;
    }

    static void AcquireGenerals(ServerPlayer *shenluxun){
        Room *room = shenluxun->getRoom();
        QStringList list = GetAvailableGenerals(shenluxun);
        qShuffle(list);

        QString general_name = list.first();
        QVariantList yings = shenluxun->tag["Yings"].toList();
        yings << general_name;
        const General *general = Sanguosha->getGeneral(general_name);
        foreach(const TriggerSkill *skill, general->getTriggerSkills()){
            room->getThread()->addTriggerSkill(skill);
        }

        shenluxun->tag["Yings"] = yings;
        shenluxun->invoke("animate", "huashen:" + general_name);

        LogMessage log;
        log.type = "#GetYing";
        log.from = shenluxun;
        log.arg = general_name;
        log.arg2 = QString::number(yings.length());
        room->sendLog(log);
    }

    static QStringList GetAvailableGenerals(ServerPlayer *shenluxun){
        QSet<QString> all = Sanguosha->getLimitedGeneralNames().toSet();
        QSet<QString> ying_set, room_set;
        QVariantList yings = shenluxun->tag["Yings"].toList();
        foreach(QVariant ying, yings)
            ying_set << ying.toString();

        Room *room = shenluxun->getRoom();
        QList<const ServerPlayer *> players = room->findChildren<const ServerPlayer *>();
        foreach(const ServerPlayer *player, players){
            room_set << player->getGeneralName();
            if(player->getGeneral2())
                room_set << player->getGeneral2Name();
        }

        static QSet<QString> banned;
        if(banned.isEmpty()){
            banned << "shenluxun" << "zuoci" << "zuocif" << "guzhielai" << "dengshizai"
                   << "caochong" << "jiangboyue" << "zhugejin" ;
        }

        return (all - banned - ying_set - room_set).toList();
    }

    static void RefreshSkills(ServerPlayer *shenluxun){
        Room *room = shenluxun->getRoom();
        QVariantList yings = shenluxun->tag["Yings"].toList();
        foreach(QVariant ying, yings){
            QString general_name = ying.toString();
            const General *general = Sanguosha->getGeneral(general_name);
            QList<const Skill *> skills = shenluxun->getVisibleSkillList();
            foreach(const Skill *skill, general->getVisibleSkillList()){
                if(skill->isLordSkill() || skill->getFrequency() == Skill::Limited
                        || skill->getFrequency() == Skill::Wake)
                    continue;

                if(!skills.contains(skill)){
                    room->acquireSkill(shenluxun, skill->objectName(), true);
                }
            }
        }
    }

    virtual void onDamaged(ServerPlayer *shenluxun, const DamageStruct &damage) const{
        int n = damage.damage, i;
        if(n == 0)
            return;

        if(shenluxun->askForSkillInvoke(objectName())){
            for(i=0; i<n; i++)
                YanShenbing::AcquireGenerals(shenluxun);
            YanShenbing::RefreshSkills(shenluxun);
        }
    }
};

class YanQiying: public TriggerSkill{
public:
    YanQiying():TriggerSkill("yanqiying"){
        events << HpRecover;
        frequency = Compulsory;
    }

    void RemoveGenral(ServerPlayer *shenluxun) const{
        Room *room = shenluxun->getRoom();
        QVariantList yings = shenluxun->tag["Yings"].toList();
        QStringList ying_list;
        foreach(QVariant ying, yings)
            ying_list << ying.toString();

        QString general_name = room->askForGeneral(shenluxun, ying_list);
        yings.removeOne(QVariant(general_name));

        LogMessage log;
        log.type = "#RemoveYing";
        log.from = shenluxun;
        log.arg = general_name;
        log.arg2 = QString::number(yings.length());
        room->sendLog(log);

        const General *general = Sanguosha->getGeneral(general_name);
        QList<const Skill *> skills = shenluxun->getVisibleSkillList();
        foreach(const Skill *skill, general->getVisibleSkillList()){
            if(skill->isLordSkill() || skill->getFrequency() == Skill::Limited
                    || skill->getFrequency() == Skill::Wake)
                continue;

            if(skills.contains(skill)){
                room->detachSkillFromPlayer(shenluxun, skill->objectName());
                if(skill->objectName() == "buqu" && shenluxun->getHp() < 0)
                    room->loseHp(shenluxun, 0);
            }
        }
        shenluxun->tag["Yings"] = yings;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *shenluxun, QVariant &data) const{
        RecoverStruct recover = data.value<RecoverStruct>();
        QVariantList yings = shenluxun->tag["Yings"].toList();
        int n = recover.recover, i;
        if(n == 0 || yings.isEmpty())
            return false;
        for(i=0; i<n; i++)
            YanQiying::RemoveGenral(shenluxun);
        YanShenbing::RefreshSkills(shenluxun);

        return false;
    }

    virtual QDialog *getDialog() const{
        static YanQiyingDialog *dialog;

        if(dialog == NULL)
            dialog = new YanQiyingDialog;

        return dialog;
    }
};

YanQiyingDialog::YanQiyingDialog()
{
    setWindowTitle(Sanguosha->translate("yanqiying"));
}

void YanQiyingDialog::popup(){
    QVariantList ying_list = Self->tag["Yings"].toList();
    QList<const General *> yings;
    foreach(QVariant ying, ying_list)
        yings << Sanguosha->getGeneral(ying.toString());

    fillGenerals(yings);

    show();
}

YanPackage::YanPackage()
    :Package("Yan")
{
    General *yanzhoutai = new General(this, "yanzhoutai", "yan", 4);
    yanzhoutai->addSkill(new YanYanhu);
    yanzhoutai->addSkill(new YanFanji);

    General *yanweiyan = new General(this, "yanweiyan", "yan", 4);
    yanweiyan->addSkill(new YanShixue);

    General *yanlejin = new General(this, "yanlejin", "yan", 4);
    yanlejin->addSkill(new YanXianfeng);
    yanlejin->addSkill(new Skill("yanbaoshang", Skill::Frequent));

    General *yanhuaxiong = new General(this, "yanhuaxiong", "yan", 4);
    yanhuaxiong->addSkill(new YanJiaozhan);

    General *yanhuatuo = new General(this, "yanhuatuo", "yan", 3);
    yanhuatuo->addSkill(new YanJiusha);
    yanhuatuo->addSkill(new YanShenyi);

    General *yanganning = new General(this, "yanganning", "yan", 4);
    yanganning->addSkill(new YanJiefu);

    General *yanguanyu = new General(this, "yanguanyu", "yan", 4);
    yanguanyu->addSkill(new YanDanqi);
    yanguanyu->addSkill("wusheng");

    General *yanzhonghui = new General(this, "yanzhonghui", "yan", 1);
    yanzhonghui->addSkill(new YanTianhui);
    yanzhonghui->addSkill(new YanYizhi);
    yanzhonghui->addSkill(new YanJifeng);

    //General *yanpuyuan = new General(this, "yanpuyuan", "yan", 3);

    General *yanlengyanju = new General(this, "yanlengyanju", "yan", 3);
    yanlengyanju->addSkill(new YanHuixuan);
    yanlengyanju->addSkill(new YanHuixuanSkip);
    yanlengyanju->addSkill(new YanDaohun);
    yanlengyanju->addSkill(new YanLongyin);
    yanlengyanju->addSkill(new YanGuiling);

    related_skills.insertMulti("yanhuixuan", "#yanhuixuan-skip");

    General *yanmifuren = new General(this, "yanmifuren", "shu", 3, false, true);
    yanmifuren->addSkill(new YanJiuzi);
    yanmifuren->addSkill(new YanTuogu);
    yanmifuren->addSkill(new YanLongtai);
    yanmifuren->addSkill(new YanToujing);
    yanmifuren->addSkill(new YanToujingDeath);

    related_skills.insertMulti("yantoujing", "#yantoujingdeath");

    General *shendiaochan = new General(this, "shendiaochan", "god", 3, false, true);
    shendiaochan->addSkill(new YanJiuse);
    shendiaochan->addSkill(new YanManwu);
    //shendiaochan->addSkill(new YanMeihuo);
    shendiaochan->addSkill(new YanShenyou);

    General *shenxiaoqiao = new General(this, "shenxiaoqiao", "god", 3, false, true);
    shenxiaoqiao->addSkill(new YanCangshan);
    shenxiaoqiao->addSkill(new YanQunwu);
    shenxiaoqiao->addSkill(new YanHuimou);
    shenxiaoqiao->addSkill(new YanFenshang);

    General *shenluxun = new General(this, "shenluxun", "god", 4);
    shenluxun->addSkill(new YanShenbing);
    shenluxun->addSkill(new YanQiying);

    addMetaObject<YanJiushaCard>();
    addMetaObject<YanJiuseCard>();
    addMetaObject<YanCangshanCard>();
    addMetaObject<YanTuoguCard>();

    skills << new YanHuixuanBuff << new YanGuilingBuff;

    patterns["sudden_strike"] = new ExpPattern("SuddenStrike");
    patterns["cover"] = new ExpPattern("Cover");
    patterns["rob"] = new ExpPattern("Rob");
    patterns["rebound"] = new ExpPattern("Rebound");
}

ADD_PACKAGE(Yan)
