#include "GameLevel.h"

GameLevel::GameLevel(QObject* parent)
    : QObject(parent) {}

GameLevel::~GameLevel() {
    qDeleteAll(m_creatures);
    m_creatures.clear();
}

void GameLevel::loadLevel1() {
    qDeleteAll(m_creatures);
    m_creatures.clear();

    Player* player = new Player(this);
    Enemy* enemy = new Enemy(this);

    m_creatures.insert("player", player);
    m_creatures.insert("enemy", enemy);

    m_usedSteps = 0;
    m_maxSteps = 5;

    connect(player, &Creature::changed, this, &GameLevel::levelChanged);
    connect(enemy, &Creature::changed, this, &GameLevel::levelChanged);

    emit levelChanged();
}

Creature* GameLevel::creature(const QString& id) const {
    return m_creatures.value(id, nullptr);
}

QList<Creature*> GameLevel::creatures() const {
    return m_creatures.values();
}

int GameLevel::usedSteps() const {
    return m_usedSteps;
}

int GameLevel::maxSteps() const {
    return m_maxSteps;
}

int GameLevel::remainingSteps() const {
    return m_maxSteps - m_usedSteps;
}

void GameLevel::consumeStep() {
    m_usedSteps++;
    emit levelChanged();
}

bool GameLevel::isWin() const {
    Creature* enemy = creature("enemy");
    return enemy && !enemy->isAlive();
}

bool GameLevel::isLose() const {
    Creature* player = creature("player");

    if (!player || !player->isAlive()) {
        return true;
    }

    return m_usedSteps >= m_maxSteps && !isWin();
}
