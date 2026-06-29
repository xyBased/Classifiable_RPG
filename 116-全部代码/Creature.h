#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

struct CodeMethod {
    QString signature;
    QString command;
};

class Creature : public QObject {
    Q_OBJECT
public:
    enum EnemyIntent {
        IntentNone,
        IntentAttack,
        IntentDefend,
        IntentHeal,
        IntentBuff,
        IntentUnknown
    };
    Q_ENUM(EnemyIntent)

protected:
    QString m_id;
    QString m_name;
    QString m_className;
    int m_hp = 1;
    int m_maxHp = 1;
    int m_atk = 1;
    int m_def = 0;
    int m_shield = 0;
    bool m_isEnemy = false;
    QVector<CodeMethod> m_methods;
    QString m_extraCodeHtml;
    QString m_description;

    EnemyIntent m_intent = IntentNone;
    int m_intentValue = 0;
    QString m_intentText;
    QString m_intentFunctionName;

    // 源码展示控制
    bool m_showAttackImpl = false;          // 是否在类外写出 attack 的函数实现（只在第一关写一次）
    QString m_baseClass = QStringLiteral("Creature");  // 继承的基类
    QString m_inheritAccess = QStringLiteral("public"); // 继承方式：public / private
    QStringList m_classDeclarations;        // 仅声明、不可点击调用的成员函数（如 onHit、previewIntent）
    QString m_classBodyNote;                // 写在类体内部的注释行
    QString m_methodAccess = QStringLiteral("public"); // 源码面板中 methods()/classDeclarations() 所属访问区
    bool m_inheritDemo = false;             // 第四关：用"基类 + 派生类"的形式演示继承权限

public:
    explicit Creature(
        QString id,
        QString name,
        QString className,
        int hp,
        int atk,
        bool isEnemy,
        QObject* parent = nullptr
    );

    virtual ~Creature() = default;

    QString id() const;
    QString name() const;
    QString className() const;
    int hp() const;
    int maxHp() const;
    int atk() const;
    int def() const;
    int shield() const;
    bool isEnemy() const;
    bool isAlive() const;
    QString description() const;
    void setDescription(const QString& description);

    void setHp(int value);
    void setMaxHp(int value);
    void addAtk(int value);
    void setAtk(int value);
    void addDef(int value);
    void setDef(int value);
    void setShield(int value);
    void clearShield();

    void attack(Creature& target);
    void takeDamage(int damage);
    void heal(int value);

    QVector<CodeMethod> methods() const;
    void addMethod(const CodeMethod& m);
    void clearMethods();

    void setExtraCodeHtml(const QString& html);
    QString extraCodeHtml() const;

    void setIntent(EnemyIntent intent, int value = 0, const QString& text = QString());
    EnemyIntent intent() const;
    int intentValue() const;
    QString intentText() const;
    void setIntentFunctionName(const QString& name);
    QString intentFunctionName() const;
    QString intentSymbol() const;

    // 源码展示控制
    void setShowAttackImpl(bool value);
    bool showAttackImpl() const;
    void setInheritInfo(const QString& baseClass, const QString& access);
    QString baseClass() const;
    QString inheritAccess() const;
    void addClassDeclaration(const QString& declaration);
    QStringList classDeclarations() const;
    void clearClassDeclarations();
    void setClassBodyNote(const QString& note);
    QString classBodyNote() const;
    void setMethodAccess(const QString& access);
    QString methodAccess() const;
    void setInheritDemo(bool value);
    bool inheritDemo() const;

    static QString baseClassCodeHtml(int level = 0);
    virtual QString classCodeHtml() const;
    virtual QString propertyText() const;
signals:
    void changed();
};
