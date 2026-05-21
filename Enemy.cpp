#include "Enemy.h"

// 原构造函数：第一关的默认敌人，行为不变
Enemy::Enemy(QObject* parent)
    : Creature("enemy", "Bug Enemy", "Enemy", 10, 2, true, parent) {
    m_methods.push_back({
        "void attack(Creature& target);",
        "enemy.attack(player);"
    });
}

// 新构造函数：不预置 methods，由 GameLevel 在加载关卡时按情景添加
Enemy::Enemy(const QString& id,
             const QString& displayName,
             int hp,
             int atk,
             QObject* parent)
    : Creature(id, displayName, "Enemy", hp, atk, true, parent) {
    // 故意不 push_back，留给 GameLevel 配置
}

QString Enemy::classCodeHtml() const {
    // 根据 id 决定点击该方法时自动输入的命令
    QString clickCmd;
    if (m_id == "enemy1")      clickCmd = "enemy1.attack(enemy2);";
    else if (m_id == "enemy2") clickCmd = "enemy2.attack(enemy1);";
    else                       clickCmd = "enemy.attack(player);";

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
    html += "    <a href=\"cmd:" + clickCmd + "\" "
                                              "style=\"color:#89DDFF; text-decoration:underline; font-weight:700;\">"
                                              "<span style='color:#82AAFF;'>void</span> attack(<span style='color:#82AAFF;'>Creature</span>&amp; target);"
                                              "</a>\n";

    html += "};";
    html += "</pre>";
    return html;
}
