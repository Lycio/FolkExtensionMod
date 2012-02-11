#ifndef TBdiyPACKAGE_H
#define TBdiyPACKAGE_H

#include "standard.h"
#include "package.h"
#include "card.h"
#include "skill.h"
#include "player.h"
#include "socket.h"

#include <QMutex>
#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCommandLinkButton>

class TBdiyPackage: public Package{
    Q_OBJECT

public:
    TBdiyPackage();
};

class DiyXianXiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DiyXianXiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};



class DiyShenZhiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DiyShenZhiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DiyKuangXiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DiyKuangXiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DiyXueJingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DiyXueJingCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DiyDouDanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DiyDouDanCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DiyJiFengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DiyJiFengCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DiyBianZhenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DiyBianZhenCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DiyXiXueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DiyXiXueCard();
    //bool diyxixue(ServerPlayer *player) const;

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(const CardUseStruct *card_use) const;
    //virtual const Card *validateInResposing(ServerPlayer *user, bool *continuable) const;
};

class DiyXiXueDialog: public QDialog{
    Q_OBJECT

public:
    static DiyXiXueDialog *GetInstance();

public slots:
    void popup();
    void selectCard(QAbstractButton *button);

private:
    DiyXiXueDialog();

    QGroupBox *createLeft();
    QGroupBox *createRight();
    QAbstractButton *createButton(const Card *card);
    QButtonGroup *group;
    QHash<QString, const Card *> map;
};

#endif // TBdiyPACKAGE_H
