#ifndef MANEUVERING2_H
#define MANEUVERING2_H



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

class Maneuvering2Package: public Package{
    Q_OBJECT

public:
    Maneuvering2Package();
};

#endif // MANEUVERING_H
