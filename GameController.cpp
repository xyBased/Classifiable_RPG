#include "GameController.h"

#include "SoundUtil.h"

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
    connect(m_sceneView, &BattleSceneView::actorHovered, this, [this](const QString& actorId) {
        if (m_commandPanel && m_commandPanel->isParameterSelectionActive()) {
            m_commandPanel->previewParameter(actorId);
        }
    });
    connect(m_sceneView, &BattleSceneView::actorHoverLeft, this, [this](const QString&) {
        if (m_commandPanel && m_commandPanel->isParameterSelectionActive()) {
            m_commandPanel->clearParameterPreview();
        }
    });
    connect(m_sceneView, &BattleSceneView::actorParameterPicked, this, [this](const QString& actorId) {
        if (m_commandPanel) m_commandPanel->commitParameter(actorId);
        if (m_sceneView) m_sceneView->setParameterPickMode(false);
    });
    connect(m_sceneView, &BattleSceneView::actorParameterPickCanceled, this, [this]() {
        if (m_commandPanel) m_commandPanel->cancelParameterSelection();
        if (m_sceneView) m_sceneView->setParameterPickMode(false);
    });
    connect(m_commandPanel, &CommandPanel::parameterSelectionChanged, this, [this](bool active, const QPoint&) {
        if (m_sceneView) m_sceneView->setParameterPickMode(active);
    });
    connect(m_infoPanel, &InfoCodePanel::commandChosen, this, &GameController::onCommandChosen);
    connect(m_infoPanel, &InfoCodePanel::classSelected, this, &GameController::onClassSelected);
    connect(m_commandPanel, &CommandPanel::commandSubmitted, this, &GameController::onCommandSubmitted);
    connect(m_level, &GameLevel::levelChanged, this, &GameController::refreshSelectedInfo);
}

void GameController::startLevel(int n) {
    if (!m_level || !m_sceneView || !m_infoPanel || !m_commandPanel) return;

    if (n < 1) n = 1;
    if (n > GameLevel::TotalLevels) n = GameLevel::TotalLevels;

    m_currentLevel = n;
    m_selectedActorId = "player";
    m_selectedClassName.clear();
    m_level->loadLevel(n);
    m_infoPanel->setClassChoices(m_level->creatures());
    m_sceneView->setSelectedActor("player");
    m_infoPanel->showCreature(m_level->creature("player"));
    m_commandPanel->clearLog();
    m_commandPanel->appendLog(QString("第 %1 关开始：%2").arg(n).arg(m_level->levelName()));
    m_commandPanel->appendLog("阅读右侧源码，输入或点击可调用的 public 函数。");

}

void GameController::restartCurrentLevel() {
    startLevel(m_currentLevel);
}

void GameController::showHint() {
}

void GameController::exitRun() {
    emit askCloseApplication();
}

int GameController::currentLevel() const {
    return m_currentLevel;
}

void GameController::onActorSelected(const QString& actorId) {
    m_selectedClassName.clear();
    m_selectedActorId = actorId;
    Creature* creature = m_level ? m_level->creature(actorId) : nullptr;
    m_infoPanel->showCreature(creature);
}

void GameController::onClassSelected(const QString& className) {
    m_selectedActorId.clear();
    m_selectedClassName = className;
    if (m_sceneView) m_sceneView->setClassHighlight(className);
}

void GameController::onCommandChosen(const QString& command) {
    if (m_commandPanel) m_commandPanel->setCommand(command);
}

void GameController::onCommandSubmitted(const QString& command) {
    if (!m_level || !m_parser || !m_commandPanel) return;
    m_commandPanel->cancelParameterSelection();
    if (m_sceneView) m_sceneView->setParameterPickMode(false);

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
        emit levelCleared(m_currentLevel, next, next <= GameLevel::TotalLevels);
        return;
    }

    if (m_level->isLose()) {
        m_commandPanel->appendLog("挑战失败。");
        emit runFailed(QString());
    }
}

void GameController::refreshSelectedInfo() {
    if (!m_level || !m_infoPanel) return;
    if (!m_selectedClassName.isEmpty()) {
        m_infoPanel->showClassCode(m_selectedClassName);
        if (m_sceneView) m_sceneView->setClassHighlight(m_selectedClassName);
        return;
    }
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
