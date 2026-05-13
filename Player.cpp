#include "Player.h"

Player::Player(QObject* parent)
    : Creature("player", "Player", "Player", 12, 3, false, parent) {

    m_methods.push_back({
        "void attack(Creature& enemy);",
        "player.attack(enemy);"
    });

    m_methods.push_back({
        "void powerAttack(Creature& enemy);",
        "player.powerAttack(enemy);"
    });

    m_methods.push_back({
        "void heal();",
        "player.heal();"
    });
}

QString Player::classCodeHtml() const {
    QString html;

    html += "<pre style='font-family:Consolas, \"Courier New\", monospace;"
            "font-size:14px; line-height:1.35; color:#EAF1FF; margin:0;"
            "white-space: pre-wrap; overflow-wrap:anywhere; word-break:break-word;'>";

    html += "<span style='color:#C792EA; font-weight:700;'>class</span> ";
    html += "<span style='color:#FFCB6B; font-weight:700;'>Player</span> : ";
    html += "<span style='color:#C792EA; font-weight:700;'>public</span> ";
    html += "<span style='color:#82AAFF;'>Creature</span> {\n";

    html += "<span style='color:#C792EA; font-weight:700;'>protected</span>:\n";
    html += "    <span style='color:#82AAFF;'>int</span> hp;\n";
    html += "    <span style='color:#82AAFF;'>int</span> atk;\n\n";

    html += "<span style='color:#C792EA; font-weight:700;'>public</span>:\n";

    html += "    <a href=\"cmd:player.attack(enemy);\" "
            "style=\"color:#89DDFF; text-decoration:underline; font-weight:700;\">"
            "<span style='color:#82AAFF;'>void</span> attack(<span style='color:#82AAFF;'>Creature</span>&amp; enemy);"
            "</a>\n";

    html += "    <a href=\"cmd:player.powerAttack(enemy);\" "
            "style=\"color:#89DDFF; text-decoration:underline; font-weight:700;\">"
            "<span style='color:#82AAFF;'>void</span> powerAttack(<span style='color:#82AAFF;'>Creature</span>&amp; enemy);"
            "</a>\n";

    html += "    <a href=\"cmd:player.heal();\" "
            "style=\"color:#89DDFF; text-decoration:underline; font-weight:700;\">"
            "<span style='color:#82AAFF;'>void</span> heal();"
            "</a>\n";

    html += "};";
    html += "</pre>";

    return html;
}



