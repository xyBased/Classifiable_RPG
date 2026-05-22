#include "Creature.h"

#include <QtGlobal>
#include <utility>

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
    m_id(std::move(id)),
    m_name(std::move(name)),
    m_className(std::move(className)),
    m_hp(hp),
    m_maxHp(hp),
    m_atk(atk),
    m_isEnemy(isEnemy),
    m_campText(isEnemy ? "Enemy" : "Player") {}

QString Creature::id() const { return m_id; }
QString Creature::name() const { return m_name; }
QString Creature::className() const { return m_className; }
int Creature::hp() const { return m_hp; }
int Creature::maxHp() const { return m_maxHp; }
int Creature::atk() const { return m_atk; }
bool Creature::isEnemy() const { return m_isEnemy; }
bool Creature::isAlive() const { return m_hp > 0; }
bool Creature::countsForWin() const { return m_countsForWin; }
bool Creature::takesTurn() const { return m_takesTurn; }
QString Creature::campText() const { return m_campText; }
QString Creature::extraPropertyText() const { return m_extraPropertyText; }
bool Creature::hasCustomClassCodeHtml() const { return !m_customClassCodeHtml.isEmpty(); }

void Creature::takeDamage(int damage) {
    if (!isAlive()) return;
    if (damage < 0) damage = 0;
    m_hp -= damage;
    if (m_hp < 0) m_hp = 0;
    emit changed();
}

void Creature::heal(int value) {
    if (!isAlive()) return;
    if (value < 0) value = 0;
    m_hp += value;
    if (m_hp > m_maxHp) m_hp = m_maxHp;
    emit changed();
}

void Creature::setHp(int value) {
    if (value < 0) value = 0;
    if (value > m_maxHp) value = m_maxHp;
    if (m_hp == value) return;
    m_hp = value;
    emit changed();
}

void Creature::setMaxHp(int value) {
    if (value < 1) value = 1;
    m_maxHp = value;
    if (m_hp > m_maxHp) m_hp = m_maxHp;
    emit changed();
}

void Creature::setAtk(int value) {
    if (value < 0) value = 0;
    if (m_atk == value) return;
    m_atk = value;
    emit changed();
}

void Creature::addAtk(int delta) {
    setAtk(m_atk + delta);
}

void Creature::setCountsForWin(bool value) {
    m_countsForWin = value;
    emit changed();
}

void Creature::setTakesTurn(bool value) {
    m_takesTurn = value;
    emit changed();
}

void Creature::setCampText(const QString& text) {
    m_campText = text;
    emit changed();
}

void Creature::setExtraPropertyText(const QString& text) {
    m_extraPropertyText = text;
    emit changed();
}

void Creature::setCustomClassCodeHtml(const QString& html) {
    m_customClassCodeHtml = html;
    emit changed();
}

QVector<CodeMethod> Creature::methods() const {
    return m_methods;
}

QString Creature::propertyText() const {
    QString text = QString(
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
                       .arg(m_campText);

    if (!m_extraPropertyText.isEmpty()) {
        text += QStringLiteral("\n") + m_extraPropertyText;
    }
    return text;
}

QString Creature::classCodeHtml() const {
    if (!m_customClassCodeHtml.isEmpty()) {
        return m_customClassCodeHtml;
    }

    QString html;
    html += "<pre style='font-family:Consolas, \"Courier New\", monospace;"
            "font-size:14px; line-height:1.35; color:#EAF1FF; margin:0;"
            "white-space: pre-wrap; overflow-wrap:anywhere; word-break:break-word;'>";

    html += QString("class %1 : public Creature {\n").arg(m_className.toHtmlEscaped());
    html += "protected:\n";
    html += "    int hp;\n";
    html += "    int atk;\n\n";
    html += "public:\n";

    for (const CodeMethod& method : m_methods) {
        html += QString("    <a href=\"cmd:%1\" style=\"color:#89DDFF; text-decoration:underline; font-weight:700;\">%2</a>\n")
        .arg(method.command.toHtmlEscaped(), method.signature.toHtmlEscaped());
    }

    html += "};";
    html += "</pre>";
    return html;
}

void Creature::addMethod(const CodeMethod& m) {
    m_methods.push_back(m);
    emit changed();
}

void Creature::clearMethods() {
    m_methods.clear();
    emit changed();
}
