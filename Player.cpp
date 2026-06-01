#include "Player.h"

static QString esc(const QString& s) { return s.toHtmlEscaped(); }
static QString linkPlayer(const QString& command, const QString& text) {
    return QString("<a href=\"cmd:%1\">%2</a>").arg(esc(command), esc(text));
}

Player::Player(QObject* parent)
    : Creature("player", "Player", "Player", 12, 3, false, parent) {
    addMethod({ "void attack(Creature& enemy);", "player.attack(enemy);" });
    addMethod({ "void powerAttack(Creature& enemy);", "player.powerAttack(enemy);" });
    addMethod({ "void heal();", "player.heal();" });
}

QString Player::classCodeHtml() const {
    QString html;
    html += "<pre>";
    html += "class Player : public Creature {\n";
    html += "public:\n";
    html += "    " + linkPlayer("player.attack(enemy);", "void attack(Creature& enemy);") + "\n";
    html += "    " + linkPlayer("player.powerAttack(enemy);", "void powerAttack(Creature& enemy);") + "\n";
    html += "    " + linkPlayer("player.heal();", "void heal();") + "\n";
    for (const CodeMethod& method : methods()) {
        if (method.command == "player.attack(enemy);" ||
            method.command == "player.powerAttack(enemy);" ||
            method.command == "player.heal();") {
            continue;
        }
        html += "    " + linkPlayer(method.command, method.signature) + "\n";
    }
    html += "};\n\n";
    html += "void Player::attack(Creature& enemy) {\n";
    html += "    enemy.takeDamage(atk);\n";
    html += "}\n\n";
    html += "void Player::powerAttack(Creature& enemy) {\n";
    html += "    enemy.takeDamage(atk * 2);\n";
    html += "}\n\n";
    html += "void Player::heal() {\n";
    html += "    Creature::heal(4);\n";
    html += "}\n";
    html += "</pre>";
    html += extraCodeHtml();
    return html;
}
