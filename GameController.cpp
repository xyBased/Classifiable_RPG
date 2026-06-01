#include "GameController.h"

#include "SoundUtil.h"

#include <QSettings>

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
    m_level->loadLevel(n);
    m_selectedActorId = "player";
    m_sceneView->setSelectedActor("player");
    m_infoPanel->showCreature(m_level->creature("player"));
    m_commandPanel->clearLog();
    m_commandPanel->appendLog(QString("第 %1 关开始").arg(n));
    m_commandPanel->appendLog("阅读右侧源码，输入或点击可调用的 public 函数。");

    if (m_currentLevel >= 5) {
        saveContinueProgress(m_currentLevel);
    }
}

void GameController::restartCurrentLevel() {
    startLevel(m_currentLevel);
}

void GameController::showHint() {
    if (!m_level || !m_commandPanel) return;
    m_commandPanel->appendLog(QString("提示：%1").arg(m_level->hintForCurrentLevel()));
}

void GameController::exitRun() {
    emit askCloseApplication();
}

int GameController::currentLevel() const {
    return m_currentLevel;
}

void GameController::onActorSelected(const QString& actorId) {
    m_selectedActorId = actorId;
    Creature* creature = m_level ? m_level->creature(actorId) : nullptr;
    m_infoPanel->showCreature(creature);
}

void GameController::onCommandChosen(const QString& command) {
    if (m_commandPanel) m_commandPanel->setCommand(command);
}

void GameController::onCommandSubmitted(const QString& command) {
    if (!m_level || !m_parser || !m_commandPanel) return;

    CommandResult result = m_parser->execute(command);

    m_commandPanel->appendLog(QStringLiteral("> ") + command);
    m_commandPanel->appendLog(result.message);

    if (result.success) {
        if (result.effect == "attack") {
            SoundUtil::playAttack();
        } else if (result.effect == "heal") {
            SoundUtil::playHeal();
        } else if (result.effect == "buff" || result.effect == "open") {
            SoundUtil::playHit();
        }
    }

    if (result.success && result.consumeStep) {
        m_level->consumeStep();
        if (!m_level->isWin() && !m_level->isLose()) {
            enemyTurn(result.sourceId);
        }
    }

    if (m_level->isWin()) {
        const int next = m_currentLevel + 1;
        if (next >= 5 && next <= GameLevel::TotalLevels) {
            saveContinueProgress(next);
        }
        emit levelCleared(m_currentLevel, next, next <= GameLevel::TotalLevels);
        return;
    }

    if (m_level->isLose()) {
        clearContinueProgress();
        m_commandPanel->appendLog("你倒下了。本次旅程结束。");
        emit runFailed(QString("你在第 %1 关失败了。新的旅程需要从头开始。").arg(m_currentLevel));
    }
}

void GameController::refreshSelectedInfo() {
    if (!m_level || !m_infoPanel) return;
    Creature* creature = m_level->creature(m_selectedActorId);
    if (!creature) creature = m_level->creature("player");
    m_infoPanel->showCreature(creature);
}

void GameController::enemyTurn(const QString& actedId) {
    if (!m_level || !m_commandPanel) return;

    Creature* player = m_level->creature("player");
    if (!player || !player->isAlive()) return;

    for (Creature* c : m_level->creatures()) {
        if (!c || !c->isEnemy() || !c->isAlive()) continue;
        if (c->id() == actedId) continue;

        switch (c->intent()) {
        case Creature::IntentAttack: {
            const int damage = c->intentValue() > 0 ? c->intentValue() : c->atk();
            if (damage > 0) {
                player->takeDamage(damage);
                SoundUtil::playHit();
                m_commandPanel->appendLog(QString("%1 发动攻击，造成 %2 点伤害。").arg(c->id()).arg(damage));
            }
            break;
        }
        case Creature::IntentHeal: {
            const int value = c->intentValue() > 0 ? c->intentValue() : 2;
            c->heal(value);
            SoundUtil::playHeal();
            m_commandPanel->appendLog(QString("%1 回复 %2 点生命。").arg(c->id()).arg(value));
            break;
        }
        case Creature::IntentBuff: {
            const int value = c->intentValue() > 0 ? c->intentValue() : 1;
            c->addAtk(value);
            c->setIntent(Creature::IntentAttack, c->atk());
            m_commandPanel->appendLog(QString("%1 强化 ATK +%2。").arg(c->id()).arg(value));
            break;
        }
        case Creature::IntentDefend:
            m_commandPanel->appendLog(QString("%1 本回合防御。").arg(c->id()));
            break;
        default:
            break;
        }

        if (!player->isAlive()) break;
    }
}

void GameController::saveContinueProgress(int level) const {
    QSettings settings;
    settings.setValue("progress/continueLevel", level);
}

void GameController::clearContinueProgress() const {
    QSettings settings;
    settings.remove("progress/continueLevel");
}
