#ifndef DISHACARDPACKAGE_H
#define DISHACARDPACKAGE_H

#include "standard.h"
#include "package.h"
#include "card.h"
#include "skill.h"

class PunctureSlash: public Slash{
    Q_OBJECT

public:
    Q_INVOKABLE PunctureSlash(Card::Suit suit, int number);
};

class BloodSlash: public Slash{
    Q_OBJECT

public:
    Q_INVOKABLE BloodSlash(Card::Suit suit, int number);
};

class PoisonPeach: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE PoisonPeach(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isAvailable(const Player *player) const;
};

class YitianJian:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE YitianJian(Card::Suit suit , int number);
};

class QixingBlade:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE QixingBlade(Card::Suit suit , int number);
};

class LuofengBow:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE LuofengBow(Card::Suit suit , int number);
};

class JiaSuo:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE JiaSuo(Card::Suit suit , int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class JiaoLiao:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE JiaoLiao(Card::Suit suit , int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class SuddenStrike: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE SuddenStrike(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
    //virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool isAvailable(const Player *player) const;
};

class Cover: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Cover(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
    //virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool isAvailable(const Player *player) const;
};

class Rebound: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Rebound(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
    //virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool isAvailable(const Player *player) const;
};

class Rob: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Rob(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
    //virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool isAvailable(const Player *player) const;
};

class IrresistibleForce:public AOE{
    Q_OBJECT

public:
    Q_INVOKABLE IrresistibleForce(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;
    virtual bool isAvailable(const Player *player) const;
};

class Plague: public Disaster{
    Q_OBJECT

public:
    Q_INVOKABLE Plague(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void takeEffect(ServerPlayer *target) const;
};

class DishaCardPackage: public Package{
    Q_OBJECT

public:
    DishaCardPackage();
};

#endif // DISHACARDPACKAGE_H
