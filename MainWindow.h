#pragma once

#include <QMainWindow>

#include "BattleSceneView.h"
#include "CommandPanel.h"
#include "InfoCodePanel.h"
#include "GameLevel.h"
#include "GameController.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    BattleSceneView* m_sceneView = nullptr;
    CommandPanel* m_commandPanel = nullptr;
    InfoCodePanel* m_infoPanel = nullptr;

    GameLevel* m_level = nullptr;
    GameController* m_controller = nullptr;

public:
    explicit MainWindow(QWidget* parent = nullptr);
};
