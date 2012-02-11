#ifndef YJ1stPACKAGE_H
#define YJ1stPACKAGE_H

#include "standard.h"
#include "package.h"
#include "card.h"
#include "skill.h"

class YJ1stPackage: public Package{
    Q_OBJECT

public:
    YJ1stPackage();
};

class YJZhuanXiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YJZhuanXiangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YJHuiZeCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YJHuiZeCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YJTanChaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YJTanChaCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YJLingYinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YJLingYinCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YJYangXiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YJYangXiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YJZhengLveCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YJZhengLveCard();

    virtual int getSuits(ServerPlayer *player) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JiehuohCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiehuohCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhangQi:public AOE{
    Q_OBJECT

public:
    Q_INVOKABLE ZhangQi(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YuQinGuZong:public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE YuQinGuZong(Card::Suit suit, int number);
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class RedFlag:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE RedFlag(Card::Suit suit , int number);
};

class JieJian: public Disaster{
    Q_OBJECT

public:
    Q_INVOKABLE JieJian(Card::Suit suit, int number);
    virtual void takeEffect(ServerPlayer *target) const;
};

class YJ1stCardPackage: public Package{
    Q_OBJECT

public:
    YJ1stCardPackage();
};

#endif // YJ1stPACKAGE_H
