#include "Enemy.h"

Enemy::Enemy(QObject* parent)
    : Creature("enemy", "Bug Enemy", "Enemy", 10, 2, true, parent) {

    m_methods.push_back({
        "void attack(Creature& target);",
        "enemy.attack(player);"
    });
}

QString Enemy::classCodeHtml() const {
    QString html;

    html += "<pre style='font-family:Consolas, \"Courier New\", monospace;"
            "font-size:14px; line-height:1.35; color:#EAF1FF; margin:0;"
            "white-space: pre-wrap; overflow-wrap:anywhere; word-break:break-word;'>";

    html += "<span style='color:#C792EA; font-weight:700;'>class</span> ";
    html += "<span style='color:#FFCB6B; font-weight:700;'>Enemy</span> : ";
    html += "<span style='color:#C792EA; font-weight:700;'>public</span> ";
    html += "<span style='color:#82AAFF;'>Creature</span> {\n";

    html += "<span style='color:#C792EA; font-weight:700;'>protected</span>:\n";
    html += "    <span style='color:#82AAFF;'>int</span> hp;\n";
    html += "    <span style='color:#82AAFF;'>int</span> atk;\n\n";

    html += "<span style='color:#C792EA; font-weight:700;'>public</span>:\n";

    html += "    <a href=\"cmd:enemy.attack(player);\" "
            "style=\"color:#89DDFF; text-decoration:underline; font-weight:700;\">"
            "<span style='color:#82AAFF;'>void</span> attack(<span style='color:#82AAFF;'>Creature</span>&amp; target);"
            "</a>\n";

    html += "};";
    html += "</pre>";

    return html;
}


