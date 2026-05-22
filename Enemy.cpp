#include "Enemy.h"

Enemy::Enemy(QObject* parent)
    : Creature("enemy", "Bug", "Enemy", 8, 2, true, parent) {
    setCampText("Enemy");
    addMethod({ "void attack(Creature& target);", "enemy.attack(player);" });
}

Enemy::Enemy(const QString& id, const QString& displayName, int hp, int atk, QObject* parent)
    : Creature(id, displayName, "Enemy", hp, atk, true, parent) {
    setCampText("Enemy");
}

QString Enemy::classCodeHtml() const {
    return Creature::classCodeHtml();
}
