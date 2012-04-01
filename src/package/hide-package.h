#ifndef HIDEPACKAGE_H
#define HIDEPACKAGE_H

#include "standard.h"

class AdouMark: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE AdouMark(Card::Suit suit, int number);

    virtual void onUninstall(ServerPlayer *player) const;
};

class HidePackage: public Package{
    Q_OBJECT

public:
    HidePackage();
};

#endif // HIDEPACKAGE_H
