#ifndef YJ3rdPACKAGE_H
#define YJ3rdPACKAGE_H

#include "standard.h"
#include "package.h"
#include "card.h"
#include "skill.h"

class YJ3rdPackage: public Package{
    Q_OBJECT

public:
    YJ3rdPackage();
};

class YjJubingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YjJubingCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YjYoujinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YjYoujinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YjRangliCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YjRangliCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YjYinjianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YjYinjianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // YJ3rdPACKAGE_H
