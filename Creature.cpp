#include "Creature.h"

#include <QtGlobal>

static QString esc(const QString& s) {
    return s.toHtmlEscaped();
}

static QString linkCmd(const QString& command, const QString& text) {
    return QString("<a href=\"cmd:%1\">%2</a>").arg(esc(command), esc(text));
}

static QString line(const QString& s) {
    return esc(s) + "\n";
}

Creature::Creature(QString id, QString name, QString className, int hp, int atk, bool isEnemy, QObject* parent)
    : QObject(parent),
      m_id(std::move(id)),
      m_name(std::move(name)),
      m_className(std::move(className)),
      m_hp(hp),
      m_maxHp(hp),
      m_atk(atk),
      m_isEnemy(isEnemy) {}

QString Creature::id() const { return m_id; }
QString Creature::name() const { return m_name; }
QString Creature::className() const { return m_className; }
int Creature::hp() const { return m_hp; }
int Creature::maxHp() const { return m_maxHp; }
int Creature::atk() const { return m_atk; }
bool Creature::isEnemy() const { return m_isEnemy; }
bool Creature::isAlive() const { return m_hp > 0; }

void Creature::setHp(int value) {
    m_hp = qBound(0, value, m_maxHp);
    emit changed();
}

void Creature::setMaxHp(int value) {
    m_maxHp = qMax(1, value);
    m_hp = qMin(m_hp, m_maxHp);
    emit changed();
}

void Creature::addAtk(int value) {
    m_atk += value;
    if (m_atk < 0) m_atk = 0;
    emit changed();
}

void Creature::setAtk(int value) {
    m_atk = qMax(0, value);
    emit changed();
}

void Creature::takeDamage(int damage) {
    if (!isAlive()) return;
    damage = qMax(0, damage);
    m_hp -= damage;
    if (m_hp < 0) m_hp = 0;
    emit changed();
}

void Creature::heal(int value) {
    if (!isAlive()) return;
    value = qMax(0, value);
    m_hp += value;
    if (m_hp > m_maxHp) m_hp = m_maxHp;
    emit changed();
}

QVector<CodeMethod> Creature::methods() const { return m_methods; }
void Creature::addMethod(const CodeMethod& m) { m_methods.push_back(m); }
void Creature::clearMethods() { m_methods.clear(); }

void Creature::setExtraCodeHtml(const QString& html) { m_extraCodeHtml = html; }
QString Creature::extraCodeHtml() const { return m_extraCodeHtml; }

void Creature::setIntent(EnemyIntent intent, int value, const QString& text) {
    m_intent = intent;
    m_intentValue = value;
    m_intentText = text;
    emit changed();
}

Creature::EnemyIntent Creature::intent() const { return m_intent; }
int Creature::intentValue() const { return m_intentValue; }

QString Creature::intentText() const {
    if (!m_intentText.isEmpty()) return m_intentText;
    switch (m_intent) {
    case IntentAttack: return QString("攻击 %1").arg(m_intentValue);
    case IntentDefend: return "防御";
    case IntentHeal: return QString("治疗 %1").arg(m_intentValue);
    case IntentBuff: return QString("强化 +%1").arg(m_intentValue);
    case IntentUnknown: return "未知";
    default: return "无";
    }
}

QString Creature::intentSymbol() const {
    switch (m_intent) {
    case IntentAttack: return "⚔";
    case IntentDefend: return "◆";
    case IntentHeal: return "✚";
    case IntentBuff: return "▲";
    case IntentUnknown: return "?";
    default: return "";
    }
}

QString Creature::propertyText() const {
    return QString("HP %1/%2    ATK %3").arg(m_hp).arg(m_maxHp).arg(m_atk);
}

QString Creature::classCodeHtml() const {
    QString html;
    html += "<pre>";
    html += line(QString("class %1 : public Creature {").arg(m_className));
    html += line("public:");
    for (const CodeMethod& method : m_methods) {
        html += "    " + linkCmd(method.command, method.signature) + "\n";
    }
    html += line("};");
    html += line("");
    html += line("void Creature::takeDamage(int damage) {");
    html += line("    hp -= max(0, damage);");
    html += line("    if (hp < 0) hp = 0;");
    html += line("}");
    html += line("");
    html += line("void Creature::heal(int value) {");
    html += line("    hp += max(0, value);");
    html += line("    if (hp > maxHp) hp = maxHp;");
    html += line("}");
    html += "</pre>";
    html += m_extraCodeHtml;
    return html;
}
