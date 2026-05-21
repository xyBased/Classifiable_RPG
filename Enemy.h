#pragma once
#include "Creature.h"

class Enemy : public Creature {
    Q_OBJECT
public:
    // 原构造函数：用于第一关的默认 Bug Enemy
    explicit Enemy(QObject* parent = nullptr);

    // 新增：用于多敌人关卡（比如第二关的 Goblin / Slime King）
    explicit Enemy(const QString& id,
                   const QString& displayName,
                   int hp,
                   int atk,
                   QObject* parent = nullptr);

    QString classCodeHtml() const override;
};
