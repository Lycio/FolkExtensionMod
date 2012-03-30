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
        if(effect.from->objectName() != player->objectName() && effect.card->isNDTrick()){
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

    addMetaObject<YanJiushaCard>();
    addMetaObject<YanJiuseCard>();
    addMetaObject<YanCangshanCard>();

    patterns["sudden_strike"] = new ExpPattern("SuddenStrike");
    patterns["cover"] = new ExpPattern("Cover");
    patterns["rob"] = new ExpPattern("Rob");
    patterns["rebound"] = new ExpPattern("Rebound");
}

ADD_PACKAGE(Yan)
