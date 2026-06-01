#pragma once

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>

#include "Creature.h"
#include "Player.h"
#include "Enemy.h"

class GameLevel : public QObject {
    Q_OBJECT
public:
    static constexpr int TotalLevels = 10;

private:
    QMap<QString, Creature*> m_creatures;
    QMap<QString, bool> m_flags;
    int m_usedSteps = 0;
    int m_maxSteps = 5;
    int m_currentLevel = 0;
    bool m_allowEnemyControl = false;
    QString m_intro;

public:
    explicit GameLevel(QObject* parent = nullptr);
    ~GameLevel();

    void loadLevel(int n);
    void loadLevel1();
    void loadLevel2();
    void loadLevel3();
    void loadLevel4();
    void loadLevel5();
    void loadLevel6();
    void loadLevel7();
    void loadLevel8();
    void loadLevel9();
    void loadLevel10();

    Creature* creature(const QString& id) const;
    QList<Creature*> creatures() const;

    int usedSteps() const;
    int maxSteps() const;
    int remainingSteps() const;
    int currentLevel() const;
    bool allowEnemyControl() const;
    QString intro() const;
    QString hintForCurrentLevel() const;

    void consumeStep();

    void setFlag(const QString& key, bool value = true);
    bool flag(const QString& key) const;
    void clearFlags();

    bool isWin() const;
    bool isLose() const;

signals:
    void levelChanged();

private:
    void resetCreatures();
    void registerCreature(const QString& id, Creature* c);
    QString htmlBlock(const QString& title, const QString& code) const;
};
