#pragma once

#include <QObject>

#include "GameLevel.h"
#include "CommandParser.h"
#include "BattleSceneView.h"
#include "InfoCodePanel.h"
#include "CommandPanel.h"

class GameController : public QObject {
    Q_OBJECT

private:
    GameLevel* m_level = nullptr;
    CommandParser* m_parser = nullptr;

    BattleSceneView* m_sceneView = nullptr;
    InfoCodePanel* m_infoPanel = nullptr;
    CommandPanel* m_commandPanel = nullptr;

    QString m_selectedActorId;

public:
    explicit GameController(QObject* parent = nullptr);

    void setup(
        GameLevel* level,
        BattleSceneView* sceneView,
        InfoCodePanel* infoPanel,
        CommandPanel* commandPanel
        );

    void startLevel1();

private slots:
    void onActorSelected(const QString& actorId);
    void onCommandChosen(const QString& command);
    void onCommandSubmitted(const QString& command);
    void refreshSelectedInfo();
};
