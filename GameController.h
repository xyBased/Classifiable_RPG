#pragma once
#include <QObject>
#include <QString>
#include "GameLevel.h"
#include "CommandParser.h"
#include "BattleSceneView.h"
#include "InfoCodePanel.h"
#include "CommandPanel.h"

class GameController : public QObject {
    Q_OBJECT
private:
    GameLevel*       m_level        = nullptr;
    CommandParser*   m_parser       = nullptr;
    BattleSceneView* m_sceneView    = nullptr;
    InfoCodePanel*   m_infoPanel    = nullptr;
    CommandPanel*    m_commandPanel = nullptr;

    QString m_selectedActorId;
    int     m_currentLevel  = 0;
    bool    m_transitioning = false;  // 过场期间禁止再触发胜负判定

    void enemyTurn(const QString& actedId);
    void playIntermission(int fromLevel, int toLevel);

public:
    explicit GameController(QObject* parent = nullptr);

    void setup(
        GameLevel* level,
        BattleSceneView* sceneView,
        InfoCodePanel* infoPanel,
        CommandPanel* commandPanel
        );

    void startLevel(int n);
    void goToNextLevel();
    void retryLevel();

private slots:
    void onActorSelected(const QString& actorId);
    void onCommandChosen(const QString& command);
    void onCommandSubmitted(const QString& command);
    void refreshSelectedInfo();
};
