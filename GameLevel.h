#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include "Creature.h"
#include "Player.h"
#include "Enemy.h"

class GameLevel : public QObject {
    Q_OBJECT
private:
    QMap<QString, Creature*> m_creatures;
    int m_usedSteps = 0;
    int m_maxSteps  = 5;
    int m_currentLevel = 0;
    bool m_allowEnemyControl = false;   // 是否允许玩家在指令框里写 enemyX.xxx()
    QString m_intro;                    // 关卡提示文案

public:
    explicit GameLevel(QObject* parent = nullptr);
    ~GameLevel();

    // 统一入口
    void loadLevel(int n);

    // 六个关卡
    void loadLevel1();
    void loadLevel2();
    void loadLevel3();
    void loadLevel4();
    void loadLevel5();
    void loadLevel6();

    Creature* creature(const QString& id) const;
    QList<Creature*> creatures() const;

    int usedSteps() const;
    int maxSteps() const;
    int remainingSteps() const;
    void consumeStep();

    int currentLevel() const;
    bool allowEnemyControl() const;
    QString intro() const;

    bool isWin() const;
    bool isLose() const;

signals:
    void levelChanged();

private:
    // 工具：清空并接好 changed -> levelChanged 的信号
    void resetCreatures();
    void registerCreature(const QString& id, Creature* c);
};
