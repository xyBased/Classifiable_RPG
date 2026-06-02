#include "Enemy.h"

Enemy::Enemy(QObject* parent)
    : Creature("bug", "bug", "Enemy", 8, 2, true, parent) {
    setIntent(IntentAttack, atk());
    setDescription(QStringLiteral("很弱的敌人，生命值和攻击力都很低。"));
}

Enemy::Enemy(const QString& id, const QString& displayName, int hp, int atk, QObject* parent)
    : Creature(id, displayName, "Enemy", hp, atk, true, parent) {
    addMethod({ "void attack(Creature& target);", id + ".attack(player);" });
    setIntent(IntentAttack, atk);
    setDescription("敌对对象。观察它的意图，并用可访问的 public 接口处理它。");
}

Enemy::Enemy(const QString& id, const QString& displayName, const QString& className, int hp, int atk, QObject* parent)
    : Creature(id, displayName, className, hp, atk, true, parent) {
    addMethod({ "void attack(Creature& target);", id + ".attack(player);" });
    setIntent(IntentAttack, atk);
    setDescription(QString("%1 类的敌对对象。观察它的意图，并用可访问的 public 接口处理它。").arg(className));
}

QString Enemy::classCodeHtml() const {
    QString html;
    html += "<pre>";
    html += QString("class %1 : public Creature {\n").arg(className());
    html += "private:\n";
    html += "    void attack(Creature& target);\n";
    html += "};\n\n";
    html += "void Enemy::attack(Creature& target) {\n";
    html += "    target.takeDamage(atk);\n";
    html += "}\n";
    html += "</pre>";
    html += extraCodeHtml();
    return html;
}
