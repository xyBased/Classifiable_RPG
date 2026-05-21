#pragma once
#include <QObject>
#include <QString>
#include <QVector>

struct CodeMethod {
    QString signature;  // 显示在右侧代码里的方法签名
    QString command;    // 点击后自动输入的命令
};

class Creature : public QObject {
    Q_OBJECT
protected:
    QString m_id;
    QString m_name;
    QString m_className;
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

    // 新增：让外部（GameLevel）能按关卡配置展示的方法
    void addMethod(const CodeMethod& m);
    void clearMethods();

signals:
    void changed();
};
