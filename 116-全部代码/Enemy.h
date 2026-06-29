#pragma once

#include "Creature.h"

class Enemy : public Creature {
    Q_OBJECT
public:
    explicit Enemy(QObject* parent = nullptr);

    explicit Enemy(
        const QString& id,
        const QString& displayName,
        int hp,
        int atk,
        QObject* parent = nullptr
    );

    explicit Enemy(
        const QString& id,
        const QString& displayName,
        const QString& className,
        int hp,
        int atk,
        QObject* parent = nullptr
    );

    QString classCodeHtml() const override;
};
