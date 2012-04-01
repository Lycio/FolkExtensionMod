#ifndef YANPACKAGE_H
#define YANPACKAGE_H

#include "standard.h"
#include "package.h"
#include "card.h"
#include "skill.h"

class YanPackage: public Package{
    Q_OBJECT

public:
    YanPackage();
};

class YanJiushaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YanJiushaCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YanJiuseCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YanJiuseCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YanCangshanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YanCangshanCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YanTuoguCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YanTuoguCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // YANPACKAGE_H
