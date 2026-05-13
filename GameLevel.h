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
    int m_maxSteps = 5;

public:
    explicit GameLevel(QObject* parent = nullptr);
    ~GameLevel();

    void loadLevel1();

    Creature* creature(const QString& id) const;
    QList<Creature*> creatures() const;

    int usedSteps() const;
    int maxSteps() const;
    int remainingSteps() const;

    void consumeStep();

    bool isWin() const;
    bool isLose() const;

signals:
    void levelChanged();
};
