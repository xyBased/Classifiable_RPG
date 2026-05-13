#include "Creature.h"

Creature::Creature(
    QString id,
    QString name,
    QString className,
    int hp,
    int atk,
    bool isEnemy,
    QObject* parent
    )
    : QObject(parent),
    m_id(id),
    m_name(name),
    m_className(className),
    m_hp(hp),
    m_maxHp(hp),
    m_atk(atk),
    m_isEnemy(isEnemy) {}

QString Creature::id() const {
    return m_id;
}

QString Creature::name() const {
    return m_name;
}

QString Creature::className() const {
    return m_className;
}

int Creature::hp() const {
    return m_hp;
}

int Creature::maxHp() const {
    return m_maxHp;
}

int Creature::atk() const {
    return m_atk;
}

bool Creature::isEnemy() const {
    return m_isEnemy;
}

bool Creature::isAlive() const {
    return m_hp > 0;
}

void Creature::takeDamage(int damage) {
    if (!isAlive()) return;

    m_hp -= damage;
    if (m_hp < 0) {
        m_hp = 0;
    }

    emit changed();
}

void Creature::heal(int value) {
    if (!isAlive()) return;

    m_hp += value;
    if (m_hp > m_maxHp) {
        m_hp = m_maxHp;
    }

    emit changed();
}

QVector<CodeMethod> Creature::methods() const {
    return m_methods;
}

QString Creature::propertyText() const {
    return QString(
               "对象名：%1\n"
               "类名：%2\n"
               "HP：%3 / %4\n"
               "ATK：%5\n"
               "阵营：%6"
               )
        .arg(m_id)
        .arg(m_className)
        .arg(m_hp)
        .arg(m_maxHp)
        .arg(m_atk)
        .arg(m_isEnemy ? "Enemy" : "Player");
}

QString Creature::classCodeHtml() const {
    QString html;

    html += "<pre>";
    html += QString("class %1 : public Creature {\n").arg(m_className);
    html += "protected:\n";
    html += "    int hp;\n";
    html += "    int atk;\n\n";
    html += "public:\n";

    for (const CodeMethod& method : m_methods) {
        html += QString(
                    "    <a href=\"cmd:%1\">%2</a>\n"
                    ).arg(method.command, method.signature);
    }

    html += "};";
    html += "</pre>";

    return html;
}
