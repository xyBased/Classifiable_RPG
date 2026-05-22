#include "Player.h"

Player::Player(QObject* parent)
    : Creature("player", "Hero", "Player", 12, 3, false, parent) {
    setCampText("Player");
    addMethod({ "void attack(Creature& enemy);", "player.attack(enemy);" });
    addMethod({ "void powerAttack(Creature& enemy);", "player.powerAttack(enemy);" });
    addMethod({ "void heal();", "player.heal();" });
}

QString Player::classCodeHtml() const {
    if (hasCustomClassCodeHtml()) {
        return Creature::classCodeHtml();
    }

    QString html;
    html += "<pre style='font-family:Consolas, \"Courier New\", monospace;"
            "font-size:14px; line-height:1.35; color:#EAF1FF; margin:0;"
            "white-space: pre-wrap; overflow-wrap:anywhere; word-break:break-word;'>";
    html += "class Player : public Creature {\n";
    html += "public:\n";
    html += "    <a href=\"cmd:player.attack(enemy);\" style=\"color:#89DDFF; text-decoration:underline; font-weight:700;\">void attack(Creature&amp; enemy);</a>\n";
    html += "    <a href=\"cmd:player.powerAttack(enemy);\" style=\"color:#89DDFF; text-decoration:underline; font-weight:700;\">void powerAttack(Creature&amp; enemy);</a>\n";
    html += "    <a href=\"cmd:player.heal();\" style=\"color:#89DDFF; text-decoration:underline; font-weight:700;\">void heal();</a>\n";
    html += "};";
    html += "</pre>";
    return html;
}
