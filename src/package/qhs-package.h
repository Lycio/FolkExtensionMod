#ifndef QHSPACKAGE_H
#define QHSPACKAGE_H

#include "package.h"
#include "card.h"
#include "skill.h"

class QHSPackage: public Package{
    Q_OBJECT

public:
    QHSPackage();
};

class FenbeihCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FenbeihCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FangshuhCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FangshuhCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhixinhCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhixinhCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FenlihCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FenlihCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhenluanhCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhenluanhCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CaijianhCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CaijianhCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class GuijihCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuijihCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JunlinghCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JunlinghCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // QHSPACKAGE_H
