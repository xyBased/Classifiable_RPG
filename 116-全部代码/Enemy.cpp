#include "Enemy.h"

#include <QStringList>

static QString escEnemy(const QString& s) { return s.toHtmlEscaped(); }

static QString linkEnemy(const QString& command, const QString& text) {
    return QString("<a href=\"cmd:%1\">%2</a>").arg(escEnemy(command), escEnemy(text));
}

static QString enemyCommandWithPlaceholders(const CodeMethod& method) {
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

// 把一行可能带行尾注释的代码拆成 (代码, 注释)，用于「先整段干净代码、再另起一行 // 诠释」。
static void splitCodeCommentE(const QString& raw, QString& code, QString& comment) {
    const QString trimmed = raw.trimmed();
    if (trimmed.startsWith("//")) { code.clear(); comment = trimmed; return; }
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

Enemy::Enemy(QObject* parent)
    : Creature("bug", "bug", "Enemy", 8, 2, true, parent) {
    setIntent(IntentAttack, atk());
    setDescription(QStringLiteral("很弱的敌人，生命值和攻击力都很低。"));
}

Enemy::Enemy(const QString& id, const QString& displayName, int hp, int atk, QObject* parent)
    : Creature(id, displayName, "Enemy", hp, atk, true, parent) {
    addMethod({ "void attack(Creature& target);", id + ".attack(player);" });
    setIntent(IntentAttack, atk);
    setDescription("敌对对象。观察它的意图，并用可访问的 public 接口处理它。");
}

Enemy::Enemy(const QString& id, const QString& displayName, const QString& className, int hp, int atk, QObject* parent)
    : Creature(id, displayName, className, hp, atk, true, parent) {
    addMethod({ "void attack(Creature& target);", id + ".attack(player);" });
    setIntent(IntentAttack, atk);
    setDescription(QString("%1 类的敌对对象。观察它的意图，并用可访问的 public 接口处理它。").arg(className));
}

QString Enemy::classCodeHtml() const {
    // 第四关：以"基类 Creature + 派生类"的形式演示继承权限。
    // attack 写在基类 public 下且可点击调用；派生类只写继承声明，类体为空。
    if (inheritDemo()) {
        QString html;
        html += "<pre>";
        html += "class Creature {\n";
        html += "public:\n";
        if (!methods().isEmpty()) {
            // 注意：methods() 按值返回临时 QVector，绝不能用引用接住 first()，
            // 否则临时对象销毁后引用悬垂、访问即崩溃。这里取值拷贝。
            const CodeMethod m = methods().first();
            html += "    " + linkEnemy(enemyCommandWithPlaceholders(m), m.signature) + "\n";
        } else {
            html += "    void attack(Creature& target);\n";
        }
        html += "};\n";
        html += "\n";
        html += QString("class %1 : %2 %3 {};\n")
            .arg(escEnemy(className()), escEnemy(inheritAccess()), escEnemy(baseClass()));
        if (!classBodyNote().isEmpty()) {
            html += escEnemy(classBodyNote()) + "\n";
        }
        html += "</pre>";
        html += extraCodeHtml();
        return html;
    }

    QString html;
    html += "<pre>";
    QStringList postComments;   // 类展示完之后，另起一行用 // 给出诠释
    // 直接继承 Creature，不再写出 Enemy 这个中间类
    html += QString("class %1 : %2 %3 {\n")
        .arg(escEnemy(className()), escEnemy(inheritAccess()), escEnemy(baseClass()));

    const bool hasDeclaredMembers = !methods().isEmpty() || !classDeclarations().isEmpty();
    if (hasDeclaredMembers || !classBodyNote().isEmpty()) {
        if (hasDeclaredMembers) {
            html += escEnemy(methodAccess()) + ":\n";
        }
        if (!classBodyNote().isEmpty()) {
            QString code, comment;
            splitCodeCommentE(classBodyNote(), code, comment);
            if (!code.isEmpty()) html += "    " + escEnemy(code) + "\n";
            if (!comment.isEmpty()) postComments << comment;
        }
        // 出现的成员函数：函数名写进类定义，且直接在 public 下，玩家可调用
        for (const CodeMethod& method : methods()) {
            html += "    " + linkEnemy(enemyCommandWithPlaceholders(method), method.signature) + "\n";
        }
        for (const QString& decl : classDeclarations()) {
            QString code, comment;
            splitCodeCommentE(decl, code, comment);
            if (!code.isEmpty()) html += "    " + escEnemy(code) + "\n";
            if (!comment.isEmpty()) postComments << comment;
        }
    }

    html += "};\n";
    // attack 的实现继承自 Creature，这里不再重复写出函数实现
    if (!postComments.isEmpty()) {
        html += "\n";
        for (const QString& c : postComments) html += escEnemy(c) + "\n";
    }
    html += "</pre>";
    html += extraCodeHtml();
    return html;
}
