#pragma once

#include <QObject>
#include <QString>
#include <QVector>

struct CodeMethod {
    QString signature;   // 显示在右侧代码里的方法签名
    QString command;     // 点击后自动输入的命令
};

class Creature : public QObject {
    Q_OBJECT

protected:
    QString m_id;          // 例如 "player" / "enemy"
    QString m_name;        // 例如 "Player" / "Bug Enemy"
    QString m_className;   // 例如 "Player" / "Enemy"
    int m_hp;
    int m_maxHp;
    int m_atk;
    bool m_isEnemy;

    QVector<CodeMethod> m_methods;

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

    void takeDamage(int damage);
    void heal(int value);

    virtual QString classCodeHtml() const;
    virtual QString propertyText() const;

    QVector<CodeMethod> methods() const;

signals:
    void changed();
};
