#ifndef HUANGJINPACKAGE_H
#define HUANGJINPACKAGE_H

#include "package.h"
#include "card.h"

class GuishuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuishuCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class WeichengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WeichengCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TianbianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TianbianCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HuangjinPackage: public Package{
    Q_OBJECT

public:
    HuangjinPackage();
};

#endif // HUANGJINPACKAGE_H
