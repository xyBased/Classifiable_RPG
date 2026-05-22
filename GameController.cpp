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
    m_level = level;
    m_sceneView = sceneView;
    m_infoPanel = infoPanel;
    m_commandPanel = commandPanel;

    m_parser = new CommandParser(m_level, this);

    m_sceneView->setLevel(m_level);

    connect(m_sceneView, &BattleSceneView::actorSelected, this, &GameController::onActorSelected);
    connect(m_infoPanel, &InfoCodePanel::commandChosen, this, &GameController::onCommandChosen);
    connect(m_commandPanel, &CommandPanel::commandSubmitted, this, &GameController::onCommandSubmitted);
    connect(m_level, &GameLevel::levelChanged, this, &GameController::refreshSelectedInfo);
}

void GameController::startLevel(int n) {
    if (!m_level || !m_sceneView || !m_infoPanel || !m_commandPanel) return;

    if (n < 1) n = 1;
    if (n > GameLevel::TotalLevels) n = GameLevel::TotalLevels;

    m_currentLevel = n;
    m_transitioning = false;

    m_level->loadLevel(n);
    m_selectedActorId = "player";
    m_infoPanel->showCreature(m_level->creature("player"));
    m_sceneView->setSelectedActor("player");

    m_commandPanel->appendLog(QString("════ 关卡 %1 / %2 开始 ════")
                                  .arg(n)
                                  .arg(GameLevel::TotalLevels));
    const QString intro = m_level->intro();
    for (const QString& line : intro.split('\n', Qt::SkipEmptyParts)) {
        m_commandPanel->appendLog(line);
    }
    m_commandPanel->appendLog("提示：点击右侧类代码里的蓝色代码可以自动输入指令。");
}

void GameController::goToNextLevel() {
    const int next = m_currentLevel + 1;
    if (next > GameLevel::TotalLevels) {
        m_transitioning = false;
        m_commandPanel->appendLog(QString("★ 恭喜你通关了所有 %1 关！★").arg(GameLevel::TotalLevels));
        m_commandPanel->appendLog("你已经把 C++ 的类规则变成了通关武器。");
        return;
    }
    startLevel(next);
}

void GameController::retryLevel() {
    startLevel(m_currentLevel);
}

void GameController::onActorSelected(const QString& actorId) {
    m_selectedActorId = actorId;
    Creature* creature = m_level ? m_level->creature(actorId) : nullptr;
    m_infoPanel->showCreature(creature);
}

void GameController::onCommandChosen(const QString& command) {
    if (m_commandPanel) {
        m_commandPanel->setCommand(command);
    }
}

void GameController::onCommandSubmitted(const QString& command) {
    if (!m_level || !m_parser || !m_commandPanel) return;

    if (m_transitioning) {
        m_commandPanel->appendLog("（关卡正在切换，请稍候…）");
        return;
    }

    CommandResult result = m_parser->execute(command);
    m_commandPanel->appendLog(QStringLiteral("> ") + command);
    m_commandPanel->appendLog(result.message);

    if (result.success && result.consumeStep) {
        m_level->consumeStep();
        if (!m_level->isWin() && !m_level->isLose()) {
            enemyTurn(result.sourceId);
        }
    }

    if (m_level->isWin()) {
        m_transitioning = true;
        m_commandPanel->appendLog("胜利！你破解了本关的类规则。");
        playIntermission(m_currentLevel, m_currentLevel + 1);
        QTimer::singleShot(2000, this, [this]() {
            goToNextLevel();
        });
    } else if (m_level->isLose()) {
        m_transitioning = true;
        m_commandPanel->appendLog("失败！步数用完或玩家阵亡。");
        m_commandPanel->appendLog("（3 秒后从本关重新开始）");
        QTimer::singleShot(3000, this, [this]() {
            retryLevel();
        });
    }
}

void GameController::refreshSelectedInfo() {
    if (!m_level || !m_infoPanel) return;
    if (m_selectedActorId.isEmpty()) return;
    Creature* creature = m_level->creature(m_selectedActorId);
    m_infoPanel->showCreature(creature);
}

void GameController::enemyTurn(const QString& actedId) {
    if (!m_level || !m_commandPanel) return;

    Creature* player = m_level->creature("player");
    if (!player || !player->isAlive()) return;

    for (Creature* c : m_level->creatures()) {
        if (!c) continue;
        if (!c->isEnemy()) continue;
        if (!c->isAlive()) continue;
        if (!c->takesTurn()) continue;
        if (c->id() == actedId) continue;
        if (!player->isAlive()) break;

        const int damage = c->atk();
        if (damage <= 0) continue;

        player->takeDamage(damage);
        m_commandPanel->appendLog(
            QString("%1.attack(player); 造成 %2 点伤害。")
                .arg(c->id())
                .arg(damage)
            );
    }
}

void GameController::playIntermission(int fromLevel, int toLevel) {
    if (!m_commandPanel) return;
    if (toLevel > GameLevel::TotalLevels) {
        return;
    }

    m_commandPanel->appendLog("─────────────────");
    m_commandPanel->appendLog(QString("⌛ 关卡 %1 完成 → 即将进入关卡 %2")
                                  .arg(fromLevel)
                                  .arg(toLevel));
    m_commandPanel->appendLog("（玩家生命值即将回满，步数清零）");
    m_commandPanel->appendLog("─────────────────");
}
