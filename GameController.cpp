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
<<<<<<< Updated upstream
            enemyTurn(result.sourceId);
=======
            // 全关卡统一规则：
            // 指令成功运行后，步数增长，并触发所有存活怪物行动；
            // 指令失败时不增长步数，也不触发怪物行动。
            enemyTurn(QString());
            processLevel10Enemy();
        }

        if (m_level->currentLevel() == 3) {
            Creature* player = m_level->creature("player");
            m_level->setFlag("level3_shield_decay_pending", player && player->shield() > 0);
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
=======
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

void GameController::processLevel10Enemy() {
    if (!m_level || !m_commandPanel) return;
    if (m_level->currentLevel() != 10) return;

    Creature* paladin = m_level->creature("paladin");
    Creature* player = m_level->creature("player");
    if (!paladin || !paladin->isAlive() || !player || !player->isAlive()) return;

    const int warrior = m_level->counter("level10_warrior", 6);
    const int healer = m_level->counter("level10_healer", 6);

    // Warrior 半身：只要其 Unit::hp 仍在就攻击 player。
    if (warrior > 0) {
        player->takeDamage(2);
        SoundUtil::playHit();
        m_commandPanel->appendLog("Paladin 的 Warrior 半身发动攻击，造成 2 点伤害。");
    }

    // Healer 半身：只要其 Unit::hp 仍在，就治疗 Warrior 子对象（最多回到 6）。
    if (healer > 0 && paladin->isAlive()) {
        int healed = warrior + 2;
        if (healed > 6) healed = 6;
        if (healed != warrior) {
            m_level->setCounter("level10_warrior", healed);
            const int total = m_level->counter("level10_healer", 6) + healed;
            paladin->setHp(total);
            SoundUtil::playHeal();
            m_commandPanel->appendLog(QString("Paladin 的 Healer 半身治疗 Warrior 子对象 +2（Warrior::hp = %1）。").arg(healed));
        }
    }
}

>>>>>>> Stashed changes
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
