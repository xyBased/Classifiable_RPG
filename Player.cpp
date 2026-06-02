#include "Player.h"

static QString esc(const QString& s) { return s.toHtmlEscaped(); }
static QString linkPlayer(const QString& command, const QString& text) {
    return QString("<a href=\"cmd:%1\">%2</a>").arg(esc(command), esc(text));
}

static QString playerCommandWithPlaceholder(const CodeMethod& method) {
    QString command = method.command;
    if (!method.signature.contains("Creature&")) return command;
    const int open = command.indexOf('(');
    const int close = command.indexOf(')', open + 1);
    if (open < 0 || close < 0 || close <= open) return command;
    return command.left(open + 1) + "?" + command.mid(close);
}

Player::Player(QObject* parent)
    : Player(12, 3, parent) {}

Player::Player(int hp, int atk, QObject* parent)
    : Creature("player", "Player", "Player", hp, atk, false, parent) {
    addMethod({ "void attack(Creature& target);", "player.attack(enemy);" });
    setDescription("玩家操控的友好角色。输入 player 可调用它的 public 接口。");
}

QString Player::classCodeHtml() const {
    QString html;
    html += "<pre>";
    html += "class Player : public Creature {\n";
    html += "public:\n";

    bool wroteAttack = false;
    for (const CodeMethod& method : methods()) {
        if (method.signature.contains("attack")) {
            html += "    " + linkPlayer(playerCommandWithPlaceholder(method), method.signature) + "\n";
            wroteAttack = true;
            break;
        }
    }
    if (!wroteAttack) {
        html += "    void attack(Creature& target);\n";
    }

    html += "};\n\n";
    html += "void Player::attack(Creature& target) {\n";
    html += "    target.takeDamage(atk);\n";
    html += "}\n";
    html += "</pre>";
    html += extraCodeHtml();
    return html;
}
