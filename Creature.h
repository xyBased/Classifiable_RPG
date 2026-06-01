#pragma once

#include <QObject>
#include <QString>
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
    bool m_isEnemy = false;
    QVector<CodeMethod> m_methods;
    QString m_extraCodeHtml;

    EnemyIntent m_intent = IntentNone;
    int m_intentValue = 0;
    QString m_intentText;

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
    bool isEnemy() const;
    bool isAlive() const;

    void setHp(int value);
    void setMaxHp(int value);
    void addAtk(int value);
    void setAtk(int value);

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
    QString intentSymbol() const;

    virtual QString classCodeHtml() const;
    virtual QString propertyText() const;

signals:
    void changed();
};
