#pragma once

#include "Creature.h"

class Player : public Creature {
    Q_OBJECT
public:
    explicit Player(QObject* parent = nullptr);
    explicit Player(int hp, int atk, QObject* parent = nullptr);
    QString classCodeHtml() const override;
};
