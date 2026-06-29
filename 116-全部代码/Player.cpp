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
    html += "class Player : " + esc(inheritAccess()) + " " + esc(baseClass()) + " {\n";
    html += "public:\n";

    if (!classBodyNote().isEmpty()) {
        html += "    " + esc(classBodyNote()) + "\n";
    }
    // 所有出现的成员函数，函数名都先写进类定义之中
    for (const CodeMethod& method : methods()) {
        html += "    " + linkPlayer(playerCommandWithPlaceholder(method), method.signature) + "\n";
    }
    for (const QString& decl : classDeclarations()) {
        html += "    " + esc(decl) + "\n";
    }

    html += "};\n";

    // attack 的函数实现只在第一关写出一次，后面的关卡不再重复
    if (showAttackImpl()) {
        html += "\n";
        html += "void Player::attack(Creature& target) {\n";
        html += "    target.takeDamage(atk);\n";
        html += "}\n";
    }

    html += "</pre>";
    html += extraCodeHtml();
    return html;
}
