#include "GameController.h"

#include "SoundUtil.h"

#include <QStringList>

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
            Creature* creature = m_level ? m_level->creature(actorId) : nullptr;
            if (creature && creature->isAlive()) {
                m_commandPanel->previewParameter(actorId);
            } else {
                m_commandPanel->clearParameterPreview();
            }
        }
    });
    connect(m_sceneView, &BattleSceneView::actorHoverLeft, this, [this](const QString&) {
        if (m_commandPanel && m_commandPanel->isParameterSelectionActive()) {
            m_commandPanel->clearParameterPreview();
        }
    });
    connect(m_sceneView, &BattleSceneView::actorParameterPicked, this, [this](const QString& actorId) {
        Creature* creature = m_level ? m_level->creature(actorId) : nullptr;
        if (!creature || !creature->isAlive()) return;
        if (m_commandPanel) m_commandPanel->commitParameter(actorId);
        if (m_sceneView) m_sceneView->setParameterPickMode(false);
    });
    connect(m_sceneView, &BattleSceneView::actorParameterPickRejected, this, [this](const QString& actorId) {
        if (m_level && m_level->currentLevel() == 2 && m_commandPanel) {
            m_commandPanel->cancelParameterSelection();
            m_commandPanel->appendLog(QString("%1 已死亡，不能选为目标。").arg(actorId));
        }
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
    m_infoPanel->setClassChoices(m_level->creatures(), n);
    m_sceneView->setSelectedActor("player");
    m_infoPanel->showCreature(m_level->creature("player"));
    m_commandPanel->setCommand(QString());
    m_commandPanel->clearLog();
    m_commandPanel->appendLog(QString("第 %1 关开始：%2").arg(n).arg(m_level->levelName()));
    m_commandPanel->appendLog("阅读右侧源码，输入或点击可调用的 public 函数。");

}

void GameController::restartCurrentLevel() {
    startLevel(m_currentLevel);
}

void GameController::showHint() {
    if (!m_level || !m_commandPanel) return;
    const QString hint = m_level->hintForCurrentLevel();
    // 在左下角的日志区域给出当前关卡的提示
    m_commandPanel->appendLog(QStringLiteral("💡 提示：") + hint);
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
    processLevel3TurnStart();
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

    if (result.success) {
        m_level->consumeStep();
        processLevel3Bombs();

        if (!m_level->isWin() && !m_level->isLose()) {
            // 全关卡统一规则：
            // 指令成功运行后，步数增长，并触发所有存活怪物行动；
            // 指令失败时不增长步数，也不触发怪物行动。
            enemyTurn(QString());
        }

        if (m_level->currentLevel() == 3) {
            Creature* player = m_level->creature("player");
            m_level->setFlag("level3_shield_decay_pending", player && player->shield() > 0);
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
    m_infoPanel->setClassChoices(m_level->creatures(), m_level->currentLevel());
    if (!m_selectedClassName.isEmpty()) {
        m_infoPanel->showClassCode(m_selectedClassName);
        if (m_sceneView) m_sceneView->setClassHighlight(m_selectedClassName);
        return;
    }
    Creature* creature = m_level->creature(m_selectedActorId);
    if (!creature) creature = m_level->creature("player");
    m_infoPanel->showCreature(creature);
}

void GameController::processLevel3TurnStart() {
    if (!m_level || !m_commandPanel) return;
    if (m_level->currentLevel() != 3) return;
    if (!m_level->flag("level3_shield_decay_pending")) return;
    Creature* player = m_level->creature("player");
    if (player && player->shield() > 0) {
        player->clearShield();
        m_commandPanel->appendLog("player 的护盾在新回合开始时消散。");
    }
    m_level->setFlag("level3_shield_decay_pending", false);
}

void GameController::processLevel3Bombs() {
    if (!m_level || !m_commandPanel) return;
    if (m_level->currentLevel() != 3) return;

    QStringList bombsToExplode;
    for (int i = 1; i <= 4; ++i) {
        const QString bombId = QString("bomb%1").arg(i);
        Creature* bomb = m_level->creature(bombId);
        if (!bomb) continue;
        if (!bomb->isAlive()) {
            bombsToExplode << bombId;
            continue;
        }
        const QString key = QString("level3_bomb_ttl_%1").arg(bombId);
        const int ttl = m_level->counter(key, 0) - 1;
        if (ttl <= 0) {
            bombsToExplode << bombId;
        } else {
            bomb->setIntent(Creature::IntentUnknown, ttl, QStringLiteral("倒计时"));
            bomb->setIntentFunctionName(QStringLiteral("countdown"));
            m_level->setCounter(key, ttl);
        }
    }

    Creature* player = m_level->creature("player");
    for (const QString& bombId : bombsToExplode) {
        Creature* bomb = m_level->creature(bombId);
        if (!bomb) continue;
        for (Creature* c : m_level->creatures()) {
            if (!c || !c->isEnemy() || !c->isAlive()) continue;
            c->takeDamage(10);
        }
        if (player) player->addDef(-2);
        m_level->removeCounter(QString("level3_bomb_ttl_%1").arg(bombId));
        m_level->removeCreature(bombId);
        SoundUtil::playHit();
        m_commandPanel->appendLog(QString("%1 析构触发，对所有敌方造成 10 点伤害，player DEF -2。").arg(bombId));
    }
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
                const int hpBefore = player->hp();
                const int shieldBefore = player->shield();
                player->takeDamage(damage);
                const int actualLost = hpBefore - player->hp();
                const int absorbed = shieldBefore - player->shield();
                SoundUtil::playHit();
                if (absorbed > 0) {
                    m_commandPanel->appendLog(QString("%1 发动攻击，原始伤害 %2，护盾抵消 %3，实际受到 %4。").arg(c->id()).arg(damage).arg(absorbed).arg(actualLost));
                } else {
                    m_commandPanel->appendLog(QString("%1 发动攻击，造成 %2 点伤害。").arg(c->id()).arg(damage));
                }
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
            c->setIntentFunctionName(QStringLiteral("attack"));
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
