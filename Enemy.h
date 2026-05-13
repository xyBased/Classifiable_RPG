#pragma once

#include "Creature.h"

class Enemy : public Creature {
    Q_OBJECT

public:
    explicit Enemy(QObject* parent = nullptr);

    QString classCodeHtml() const override;
};
