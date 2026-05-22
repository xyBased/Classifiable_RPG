#pragma once

#include <QObject>
#include <QString>
#include <QVector>

struct CodeMethod {
    QString signature; // 显示在右侧代码里的方法签名
    QString command;   // 点击后自动输入的命令
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

    // 新增：把“是否显示为右侧对象 / 是否算胜利目标 / 是否会自动攻击”拆开。
    // 这样宝石、箱子、法术槽等也可以被点击查看代码，但不会被当成必须击杀的敌人。
    bool m_countsForWin = true;
    bool m_takesTurn = true;
    QString m_campText;

    QVector<CodeMethod> m_methods;
    QString m_customClassCodeHtml;
    QString m_extraPropertyText;

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

    bool countsForWin() const;
    bool takesTurn() const;
    QString campText() const;
    QString extraPropertyText() const;
    bool hasCustomClassCodeHtml() const;

    void takeDamage(int damage);
    void heal(int value);

    void setHp(int value);
    void setMaxHp(int value);
    void setAtk(int value);
    void addAtk(int delta);
    void setCountsForWin(bool value);
    void setTakesTurn(bool value);
    void setCampText(const QString& text);
    void setExtraPropertyText(const QString& text);
    void setCustomClassCodeHtml(const QString& html);

    virtual QString classCodeHtml() const;
    virtual QString propertyText() const;
    QVector<CodeMethod> methods() const;

    void addMethod(const CodeMethod& m);
    void clearMethods();

signals:
    void changed();
};
