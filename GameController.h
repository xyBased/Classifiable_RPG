#pragma once

#include <QObject>
#include <QString>

#include "BattleSceneView.h"
#include "CommandPanel.h"
#include "CommandParser.h"
#include "GameLevel.h"
#include "InfoCodePanel.h"

class GameController : public QObject {
    Q_OBJECT
private:
    GameLevel* m_level = nullptr;
    CommandParser* m_parser = nullptr;
    BattleSceneView* m_sceneView = nullptr;
    InfoCodePanel* m_infoPanel = nullptr;
    CommandPanel* m_commandPanel = nullptr;

    QString m_selectedActorId = "player";
    int m_currentLevel = 0;

    void enemyTurn(const QString& actedId);
    void saveContinueProgress(int level) const;
    void clearContinueProgress() const;

public:
    explicit GameController(QObject* parent = nullptr);

    void setup(
        GameLevel* level,
        BattleSceneView* sceneView,
        InfoCodePanel* infoPanel,
        CommandPanel* commandPanel
    );

    void startLevel(int n);
    void restartCurrentLevel();
    void showHint();
    void exitRun();

    int currentLevel() const;

signals:
    void levelCleared(int clearedLevel, int nextLevel, bool hasNextLevel);
    void runFailed(const QString& message);
    void returnedToMenu();
    void askCloseApplication();

private slots:
    void onActorSelected(const QString& actorId);
    void onCommandChosen(const QString& command);
    void onCommandSubmitted(const QString& command);
    void refreshSelectedInfo();
};
