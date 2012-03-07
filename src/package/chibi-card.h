#ifndef CHIBICARDPACKAGE_H
#define CHIBICARDPACKAGE_H

#include "standard.h"
#include "package.h"
#include "card.h"
#include "skill.h"
#include "maneuvering.h"

//Basic
class WindSlash: public NatureSlash{
    Q_OBJECT

public:
    Q_INVOKABLE WindSlash(Card::Suit suit, int number);
};

class Crisp: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Crisp(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;

    static bool IsAvailable(const Player *player);

    virtual bool isAvailable(const Player *player) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

//Equip
class BaguaSword: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE BaguaSword(Card::Suit suit, int number);
};

class Catapult:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Catapult(Card::Suit suit = Heart, int number = 5);
};

class Wall: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Wall(Card::Suit suit, int number);
};

class BlackArmor:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE BlackArmor(Card::Suit suit, int number);

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

protected:
    bool attach_skill;
};

class Boat: public DefensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Boat(Card::Suit suit, int number);

    virtual void onInstall(ServerPlayer *player) const;
    virtual QString getEffectPath(bool is_male) const;

private:
    TriggerSkill *boatskill;
};

//Trick
class Raid: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Raid(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Steal: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Steal(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Flooding:public AOE{
    Q_OBJECT

public:
    Q_INVOKABLE Flooding(Card::Suit suit, int number);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;
    virtual bool isAvailable(const Player *player) const;
};

class Sucker:public DelayedTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Sucker(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void takeEffect(ServerPlayer *target) const;
};

class ChibiCardPackage: public Package{
    Q_OBJECT

public:
    ChibiCardPackage();
};

#endif // CHIBICARDPACKAGE_H
