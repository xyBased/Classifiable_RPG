#include "GameController.h"

GameController::GameController(QObject* parent)
    : QObject(parent) {}

void GameController::setup(
    GameLevel* level,
    BattleSceneView* sceneView,
    InfoCodePanel* infoPanel,
    CommandPanel* commandPanel
    ) {
    m_level = level;
    m_sceneView = sceneView;
    m_infoPanel = infoPanel;
    m_commandPanel = commandPanel;

    m_parser = new CommandParser(m_level, this);

    m_sceneView->setLevel(m_level);

    connect(m_sceneView, &BattleSceneView::actorSelected,
            this, &GameController::onActorSelected);

    connect(m_infoPanel, &InfoCodePanel::commandChosen,
            this, &GameController::onCommandChosen);

    connect(m_commandPanel, &CommandPanel::commandSubmitted,
            this, &GameController::onCommandSubmitted);

    connect(m_level, &GameLevel::levelChanged,
            this, &GameController::refreshSelectedInfo);
}

void GameController::startLevel1() {
    m_level->loadLevel1();
    m_selectedActorId = "player";

    m_infoPanel->showCreature(m_level->creature("player"));
    m_commandPanel->appendLog("关卡 1：在 5 步内击败 enemy。");
    m_commandPanel->appendLog("提示：点击右侧类代码中的方法可以自动输入指令。");
}

void GameController::onActorSelected(const QString& actorId) {
    m_selectedActorId = actorId;

    Creature* creature = m_level->creature(actorId);
    m_infoPanel->showCreature(creature);
}

void GameController::onCommandChosen(const QString& command) {
    m_commandPanel->setCommand(command);
}

void GameController::onCommandSubmitted(const QString& command) {
    CommandResult result = m_parser->execute(command);

    m_commandPanel->appendLog("> " + command);
    m_commandPanel->appendLog(result.message);

    if (result.success && result.consumeStep) {
        m_level->consumeStep();
        if (!m_level->isWin() && !m_level->isLose()) {
            enemyTurn();
        }
    }

    if (m_level->isWin()) {
        m_commandPanel->appendLog("胜利！你在有限步数内击败了敌人。");
    } else if (m_level->isLose()) {
        m_commandPanel->appendLog("失败！步数用完或玩家阵亡。");
    }
}

void GameController::refreshSelectedInfo() {
    if (m_selectedActorId.isEmpty()) return;

    Creature* creature = m_level->creature(m_selectedActorId);
    m_infoPanel->showCreature(creature);
}
void GameController::enemyTurn() {
    if (!m_level) return;

    Creature* enemy = m_level->creature("enemy");
    Creature* player = m_level->creature("player");

    if (!enemy || !player) return;
    if (!enemy->isAlive()) return;  // 敌人死了就不行动
    if (!player->isAlive()) return; // 玩家死了就结束

    int damage = enemy->atk();
    player->takeDamage(damage);

    m_commandPanel->appendLog(
        QString("enemy.attack(player); 造成 %1 点伤害。").arg(damage)
        );

    // 这里不消耗步数，步数只统计玩家操作
}
