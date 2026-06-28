#include "Creature.h"

#include <QtGlobal>
#include <QStringList>

static QString esc(const QString& s) {
    return s.toHtmlEscaped();
}

static QString linkCmd(const QString& command, const QString& text) {
    return QString("<a href=\"cmd:%1\">%2</a>").arg(esc(command), esc(text));
}

static QString line(const QString& s) {
    return esc(s) + "\n";
}

// 把一行可能带行尾注释的代码拆成 (代码, 注释) 两部分，用于「先展示整段干净代码、
// 再另起一行用 // 给出诠释」的排版：
//   - 整行就是注释（trim 后以 // 开头）：code 为空，comment 为整行。
//   - 行尾挂着 // 注释：code 为注释前的代码（去掉尾随空白），comment 为 // 起的部分。
//   - 没有注释：code 为原文，comment 为空。
static void splitCodeComment(const QString& raw, QString& code, QString& comment) {
    const QString trimmed = raw.trimmed();
    if (trimmed.startsWith("//")) {
        code.clear();
        comment = trimmed;
        return;
    }
    const int idx = raw.indexOf("//");
    if (idx > 0) {
        code = raw.left(idx);
        while (code.endsWith(' ') || code.endsWith('\t')) code.chop(1);
        comment = raw.mid(idx).trimmed();
    } else {
        code = raw;
        comment.clear();
    }
}

static QString commandWithPlaceholders(const CodeMethod& method) {
    QString command = method.command;
    const QString signature = method.signature;
    const bool hasObjectParam = signature.contains("Creature&") || signature.contains("T&");
    if (!hasObjectParam) return command;

    const int open = command.indexOf('(');
    const int close = command.indexOf(')', open + 1);
    if (open < 0 || close < 0 || close <= open) return command;

    const QString argsText = command.mid(open + 1, close - open - 1).trimmed();
    if (argsText.isEmpty()) return command;

    const int argCount = argsText.split(',', Qt::SkipEmptyParts).size();
    if (argCount <= 0) return command;

    QStringList placeholders;
    for (int i = 0; i < argCount; ++i) placeholders << "?";
    return command.left(open + 1) + placeholders.join(", ") + command.mid(close);
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
int Creature::def() const { return m_def; }
int Creature::shield() const { return m_shield; }
bool Creature::isEnemy() const { return m_isEnemy; }
bool Creature::isAlive() const { return m_hp > 0; }

QString Creature::description() const {
    if (!m_description.isEmpty()) return m_description;
    if (m_isEnemy) {
        return QString("敌对对象，会按照当前意图行动。查看类代码可了解它的 public 接口。");
    }
    return QString("友好对象，玩家通常通过输入这个名称来调用它的 public 接口。");
}

void Creature::setDescription(const QString& description) {
    m_description = description;
}

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

void Creature::addDef(int value) {
    m_def += value;
    if (m_def < 0) m_def = 0;
    emit changed();
}

void Creature::setDef(int value) {
    m_def = qMax(0, value);
    emit changed();
}

void Creature::setShield(int value) {
    m_shield = qMax(0, value);
    emit changed();
}

void Creature::clearShield() {
    if (m_shield == 0) return;
    m_shield = 0;
    emit changed();
}

void Creature::attack(Creature& target) {
    if (!isAlive() || !target.isAlive()) return;
    target.takeDamage(m_atk);
}

void Creature::takeDamage(int damage) {
    if (!isAlive()) return;
    damage = qMax(0, damage);
    if (m_shield > 0 && damage > 0) {
        const int absorbed = qMin(m_shield, damage);
        m_shield -= absorbed;
        damage -= absorbed;
    }
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

void Creature::setIntentFunctionName(const QString& name) {
    m_intentFunctionName = name.trimmed();
    emit changed();
}

QString Creature::intentFunctionName() const {
    if (!m_intentFunctionName.isEmpty()) return m_intentFunctionName;
    switch (m_intent) {
    case IntentAttack: return QStringLiteral("attack");
    case IntentDefend: return QStringLiteral("defend");
    case IntentHeal: return QStringLiteral("heal");
    case IntentBuff: return QStringLiteral("empower");
    case IntentUnknown: return QStringLiteral("unknown");
    default: return QString();
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

void Creature::setShowAttackImpl(bool value) { m_showAttackImpl = value; }
bool Creature::showAttackImpl() const { return m_showAttackImpl; }

void Creature::setInheritInfo(const QString& baseClass, const QString& access) {
    m_baseClass = baseClass;
    m_inheritAccess = access;
}
QString Creature::baseClass() const { return m_baseClass; }
QString Creature::inheritAccess() const { return m_inheritAccess; }

void Creature::addClassDeclaration(const QString& declaration) { m_classDeclarations.push_back(declaration); }
QStringList Creature::classDeclarations() const { return m_classDeclarations; }
void Creature::clearClassDeclarations() { m_classDeclarations.clear(); }

void Creature::setClassBodyNote(const QString& note) { m_classBodyNote = note; }
QString Creature::classBodyNote() const { return m_classBodyNote; }

void Creature::setMethodAccess(const QString& access) {
    const QString normalized = access.trimmed().toLower();
    m_methodAccess = (normalized == "protected" || normalized == "private")
        ? normalized
        : QStringLiteral("public");
}
QString Creature::methodAccess() const { return m_methodAccess; }

void Creature::setInheritDemo(bool value) { m_inheritDemo = value; }
bool Creature::inheritDemo() const { return m_inheritDemo; }

QString Creature::propertyText() const {
    return QString("HP %1/%2    ATK %3    DEF %4    SHIELD %5")
        .arg(m_hp).arg(m_maxHp).arg(m_atk).arg(m_def).arg(m_shield);
}

QString Creature::baseClassCodeHtml(int level) {
    const bool attackPublic = false;          // 旧的「继承权限」演示已移除
    // 基类给出 virtual onHit 作为分发入口，供这些关卡的子类重写
    const bool showOnHit = (level == 4 || level == 6 || level == 9);
    QString html;
    html += "<pre>";
    html += line("class Creature {");
    if (attackPublic) {
        html += line("public:");
        html += line("    void attack(Creature& target);");
        html += line("protected:");
        html += line("    int hp;");
        html += line("    int atk;");
        html += line("    void takeDamage(int damage);");
    } else {
        if (showOnHit) {
            html += line("public:");
            html += line("    virtual void onHit(Creature& attacker, int damage);");
        }
        html += line("protected:");
        html += line("    int hp;");
        html += line("    int atk;");
        html += line("    void attack(Creature& target);");
        html += line("    void takeDamage(int damage);");
    }
    html += line("};");
    html += line("");
    html += line("void Creature::attack(Creature& target) {");
    html += line("    target.takeDamage(atk);");
    html += line("}");
    html += line("");
    html += line("void Creature::takeDamage(int damage) {");
    html += line("    hp -= max(0, damage);");
    html += line("    if (hp < 0) hp = 0;");
    html += line("}");
    if (showOnHit) {
        html += line("");
        html += line("void Creature::onHit(Creature& attacker, int damage) {");
        html += line("    hp -= damage;");
        html += line("}");
        html += line("// 基类的默认实现：直接按伤害扣血。子类可以重写 onHit 改变这一行为。");
    }
    html += "</pre>";
    return html;
}

QString Creature::classCodeHtml() const {
    if (m_className == QStringLiteral("Creature") && m_id == QStringLiteral("king")) {
        return Creature::baseClassCodeHtml(4) + m_extraCodeHtml;
    }

    QString html;
    QStringList postComments;   // 整个类展示完之后，另起一行用 // 给出的诠释
    html += "<pre>";
    html += line(QString("class %1 : %2 %3 {").arg(m_className, m_inheritAccess, m_baseClass));

    const bool hasDeclaredMembers = !m_methods.isEmpty() || !m_classDeclarations.isEmpty();
    if (hasDeclaredMembers || !m_classBodyNote.isEmpty()) {
        if (hasDeclaredMembers) {
            html += line(m_methodAccess + ":");
        }
        if (!m_classBodyNote.isEmpty()) {
            QString code, comment;
            splitCodeComment(m_classBodyNote, code, comment);
            if (!code.isEmpty()) html += "    " + esc(code) + "\n";
            if (!comment.isEmpty()) postComments << comment;
        }
        for (const CodeMethod& method : m_methods) {
            html += "    " + linkCmd(commandWithPlaceholders(method), method.signature) + "\n";
        }
        for (const QString& decl : m_classDeclarations) {
            QString code, comment;
            splitCodeComment(decl, code, comment);
            if (!code.isEmpty()) html += "    " + esc(code) + "\n";
            if (!comment.isEmpty()) postComments << comment;
        }
    }
    html += line("};");
    // 类体内不再逐行挂注释；统一在类展示完之后另起一行用 // 给出诠释
    if (!postComments.isEmpty()) {
        html += line("");
        for (const QString& c : postComments) html += line(c);
    }
    html += "</pre>";
    html += m_extraCodeHtml;
    return html;
}
