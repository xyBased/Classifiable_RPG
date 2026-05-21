#include "GameController.h"
#include <QTimer>

GameController::GameController(QObject* parent)
    : QObject(parent) {}

void GameController::setup(
    GameLevel* level,
    BattleSceneView* sceneView,
    InfoCodePanel* infoPanel,
    CommandPanel* commandPanel
    ) {
    m_level        = level;
    m_sceneView    = sceneView;
    m_infoPanel    = infoPanel;
    m_commandPanel = commandPanel;
    m_parser       = new CommandParser(m_level, this);

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

void GameController::startLevel(int n) {
    m_currentLevel  = n;
    m_transitioning = false;

    m_level->loadLevel(n);
    m_selectedActorId = "player";
    m_infoPanel->showCreature(m_level->creature("player"));
    m_sceneView->setSelectedActor("player");

    // 关卡引入文本
    m_commandPanel->appendLog(QString("════ 关卡 %1 开始 ════").arg(n));
    const QString intro = m_level->intro();
    for (const QString& line : intro.split('\n', Qt::SkipEmptyParts)) {
        m_commandPanel->appendLog(line);
    }
    m_commandPanel->appendLog("提示：点击右侧类代码里的方法可以自动输入指令。");
}

void GameController::goToNextLevel() {
    int next = m_currentLevel + 1;
    if (next > 6) {
        m_commandPanel->appendLog("★ 恭喜你通关了所有 6 关！★");
        return;
    }
    startLevel(next);
}

void GameController::retryLevel() {
    startLevel(m_currentLevel);
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
    if (m_transitioning) {
        m_commandPanel->appendLog("（关卡正在切换，请稍候…）");
        return;
    }

    CommandResult result = m_parser->execute(command);
    m_commandPanel->appendLog("> " + command);
    m_commandPanel->appendLog(result.message);

    if (result.success && result.consumeStep) {
        m_level->consumeStep();
        if (!m_level->isWin() && !m_level->isLose()) {
            enemyTurn(result.sourceId);
        }
    }

    if (m_level->isWin()) {
        m_transitioning = true;
        m_commandPanel->appendLog("胜利！你击败了所有敌人。");
        playIntermission(m_currentLevel, m_currentLevel + 1);

        // 2 秒后进入下一关（玩家血量、步数都会因新建关卡而重置）
        QTimer::singleShot(2000, this, [this]() { goToNextLevel(); });
    } else if (m_level->isLose()) {
        m_transitioning = true;
        m_commandPanel->appendLog("失败！步数用完或玩家阵亡。");
        m_commandPanel->appendLog("（3 秒后从本关重新开始）");
        QTimer::singleShot(3000, this, [this]() { retryLevel(); });
    }
}

void GameController::refreshSelectedInfo() {
    if (m_selectedActorId.isEmpty()) return;
    Creature* creature = m_level->creature(m_selectedActorId);
    m_infoPanel->showCreature(creature);
}

void GameController::enemyTurn(const QString& actedId) {
    if (!m_level) return;
    Creature* player = m_level->creature("player");
    if (!player) return;

    for (Creature* c : m_level->creatures()) {
        if (!c->isEnemy())          continue;
        if (!c->isAlive())          continue;
        if (c->id() == actedId)     continue;   // 这只这回合被玩家"借走"
        if (!player->isAlive())     break;

        int damage = c->atk();
        player->takeDamage(damage);
        m_commandPanel->appendLog(
            QString("%1.attack(player); 造成 %2 点伤害。")
                .arg(c->id()).arg(damage));
    }
}

// ===== 关卡间过场（占位框架，方便你后续扩展） =====
void GameController::playIntermission(int fromLevel, int toLevel) {
    if (toLevel > 6) {
        return; // 通关，不放过场
    }
    m_commandPanel->appendLog("─────────────────");
    m_commandPanel->appendLog(QString("⌛ 关卡 %1 完成 → 即将进入关卡 %2")
                                  .arg(fromLevel).arg(toLevel));
    m_commandPanel->appendLog("（玩家生命值即将回满，步数清零）");
    // 想加更丰富的过场（图片、对白、按下任意键继续等），
    // 在这里弹一个新窗口或者切到一个 IntermissionView 即可。
    m_commandPanel->appendLog("─────────────────");
}
