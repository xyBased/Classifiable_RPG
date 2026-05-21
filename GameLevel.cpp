#include "GameLevel.h"

GameLevel::GameLevel(QObject* parent)
    : QObject(parent) {}

GameLevel::~GameLevel() {
    qDeleteAll(m_creatures);
    m_creatures.clear();
}

// ===== 总入口 =====
void GameLevel::loadLevel(int n) {
    switch (n) {
    case 1: loadLevel1(); break;
    case 2: loadLevel2(); break;
    case 3: loadLevel3(); break;
    case 4: loadLevel4(); break;
    case 5: loadLevel5(); break;
    case 6: loadLevel6(); break;
    default: loadLevel1(); break;
    }
}

// ===== 工具 =====
void GameLevel::resetCreatures() {
    qDeleteAll(m_creatures);
    m_creatures.clear();
    m_usedSteps = 0;
}

void GameLevel::registerCreature(const QString& id, Creature* c) {
    m_creatures.insert(id, c);
    connect(c, &Creature::changed, this, &GameLevel::levelChanged);
}

// ===== 第一关（保持原行为）=====
void GameLevel::loadLevel1() {
    resetCreatures();

    Player* player = new Player(this);
    Enemy*  enemy  = new Enemy(this);                // 默认 Bug Enemy
    registerCreature("player", player);
    registerCreature("enemy",  enemy);

    m_maxSteps = 5;
    m_currentLevel = 1;
    m_allowEnemyControl = false;                     // 第一关禁止玩家控制敌人
    m_intro = "关卡 1：在 5 步内击败 enemy。";

    emit levelChanged();
}

// ===== 第二关：本次重点 =====
void GameLevel::loadLevel2() {
    resetCreatures();

    Player* player = new Player(this);

    // ⭐ 数值更新：让默认玩家(hp=12, atk=3)也无法正面打赢
    Enemy* e1 = new Enemy("enemy1", "Goblin",     3, 7,  this);
    Enemy* e2 = new Enemy("enemy2", "Slime King", 7, 20, this);

    e1->addMethod({
        "void attack(Creature& target);",
        "enemy1.attack(enemy2);"
    });
    e2->addMethod({
        "void attack(Creature& target);",
        "enemy2.attack(enemy1);"
    });

    registerCreature("player", player);
    registerCreature("enemy1", e1);
    registerCreature("enemy2", e2);

    m_maxSteps = 8;
    m_currentLevel = 2;
    m_allowEnemyControl = true;
    m_intro =
        "关卡 2：屏幕上有两只敌人。\n"
        "正常硬拼一回合就会死。\n"
        "提示：仔细看 Enemy 类——attack() 在 public 区段。\n"
        "也就是说，外部代码可以这样写：enemy1.attack(enemy2);";

    emit levelChanged();
}

// ===== 第 3~6 关：占位，等你后续实现 =====
void GameLevel::loadLevel3() {
    resetCreatures();
    Player* p = new Player(this);
    Enemy*  e = new Enemy(this);
    registerCreature("player", p);
    registerCreature("enemy",  e);
    m_maxSteps = 5;
    m_currentLevel = 3;
    m_allowEnemyControl = false;
    m_intro = "关卡 3：占位关卡，等待设计。";
    emit levelChanged();
}

void GameLevel::loadLevel4() {
    resetCreatures();
    Player* p = new Player(this);
    Enemy*  e = new Enemy(this);
    registerCreature("player", p);
    registerCreature("enemy",  e);
    m_maxSteps = 5;
    m_currentLevel = 4;
    m_allowEnemyControl = false;
    m_intro = "关卡 4：占位关卡，等待设计。";
    emit levelChanged();
}

void GameLevel::loadLevel5() {
    resetCreatures();
    Player* p = new Player(this);
    Enemy*  e = new Enemy(this);
    registerCreature("player", p);
    registerCreature("enemy",  e);
    m_maxSteps = 5;
    m_currentLevel = 5;
    m_allowEnemyControl = false;
    m_intro = "关卡 5：占位关卡，等待设计。";
    emit levelChanged();
}

void GameLevel::loadLevel6() {
    resetCreatures();
    Player* p = new Player(this);
    Enemy*  e = new Enemy(this);
    registerCreature("player", p);
    registerCreature("enemy",  e);
    m_maxSteps = 5;
    m_currentLevel = 6;
    m_allowEnemyControl = false;
    m_intro = "关卡 6：占位关卡，等待设计。";
    emit levelChanged();
}

// ===== 查询 =====
Creature* GameLevel::creature(const QString& id) const {
    return m_creatures.value(id, nullptr);
}
QList<Creature*> GameLevel::creatures() const { return m_creatures.values(); }

int GameLevel::usedSteps()      const { return m_usedSteps; }
int GameLevel::maxSteps()       const { return m_maxSteps; }
int GameLevel::remainingSteps() const { return m_maxSteps - m_usedSteps; }
int GameLevel::currentLevel()   const { return m_currentLevel; }
bool GameLevel::allowEnemyControl() const { return m_allowEnemyControl; }
QString GameLevel::intro()      const { return m_intro; }

void GameLevel::consumeStep() {
    m_usedSteps++;
    emit levelChanged();
}

// ===== 通用胜负判定（不再写死 "enemy"）=====
bool GameLevel::isWin() const {
    bool anyEnemy = false;
    for (Creature* c : m_creatures) {
        if (c->isEnemy()) {
            anyEnemy = true;
            if (c->isAlive()) return false;
        }
    }
    return anyEnemy;          // 所有敌人都倒下才算赢
}

bool GameLevel::isLose() const {
    Creature* player = creature("player");
    if (!player || !player->isAlive()) return true;
    return m_usedSteps >= m_maxSteps && !isWin();
}
