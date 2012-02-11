#ifndef GHOST_H
#define GHOST_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

class GhostPackage : public Package{
    Q_OBJECT

public:
    GhostPackage();
};

class ShouyeGhostCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShouyeGhostCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JiehuohCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiehuohCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // GHOST_H
