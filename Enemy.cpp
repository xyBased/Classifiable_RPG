#include "Enemy.h"

static QString escEnemy(const QString& s) { return s.toHtmlEscaped(); }
static QString linkEnemy(const QString& command, const QString& text) {
    return QString("<a href=\"cmd:%1\">%2</a>").arg(escEnemy(command), escEnemy(text));
}

Enemy::Enemy(QObject* parent)
    : Creature("enemy", "Bug Enemy", "Enemy", 10, 2, true, parent) {
    addMethod({ "void attack(Creature& target);", "enemy.attack(player);" });
    setIntent(IntentAttack, atk());
}

Enemy::Enemy(const QString& id, const QString& displayName, int hp, int atk, QObject* parent)
    : Creature(id, displayName, "Enemy", hp, atk, true, parent) {
    addMethod({ "void attack(Creature& target);", id + ".attack(player);" });
    setIntent(IntentAttack, atk);
}

Enemy::Enemy(const QString& id, const QString& displayName, const QString& className, int hp, int atk, QObject* parent)
    : Creature(id, displayName, className, hp, atk, true, parent) {
    addMethod({ "void attack(Creature& target);", id + ".attack(player);" });
    setIntent(IntentAttack, atk);
}

QString Enemy::classCodeHtml() const {
    QString html;
    html += "<pre>";
    html += QString("class %1 : public Creature {\n").arg(className());
    html += "public:\n";
    for (const CodeMethod& method : methods()) {
        html += "    " + linkEnemy(method.command, method.signature) + "\n";
    }
    html += "};\n\n";
    html += "void Enemy::attack(Creature& target) {\n";
    html += "    target.takeDamage(atk);\n";
    html += "}\n\n";
    html += "Intent Enemy::previewIntent() const {\n";
    html += "    return nextIntent;\n";
    html += "}\n";
    html += "</pre>";
    html += extraCodeHtml();
    return html;
}
