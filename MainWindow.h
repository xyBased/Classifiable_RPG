#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QMenu>

#include "BattleSceneView.h"
#include "CommandPanel.h"
#include "GameController.h"
#include "GameLevel.h"
#include "InfoCodePanel.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    QStackedWidget* m_stack = nullptr;
    QWidget* m_startPage = nullptr;
    QWidget* m_gamePage = nullptr;
    QWidget* m_transitionPage = nullptr;

    QLabel* m_startHintLabel = nullptr;
    QPushButton* m_continueButton = nullptr;

    QLabel* m_transitionTitleLabel = nullptr;
    QLabel* m_transitionSubtitleLabel = nullptr;
    QPushButton* m_transitionNextButton = nullptr;
    QPushButton* m_transitionExitButton = nullptr;

    QPushButton* m_menuButton = nullptr;

    BattleSceneView* m_sceneView = nullptr;
    CommandPanel* m_commandPanel = nullptr;
    InfoCodePanel* m_infoPanel = nullptr;
    GameLevel* m_level = nullptr;
    GameController* m_controller = nullptr;

    int m_pendingNextLevel = 1;
    int m_closeSaveLevel = 0;

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QWidget* createStartPage();
    QWidget* createGamePage();
    QWidget* createTransitionPage();

    int savedContinueLevel() const;
    void refreshStartMenu();
    void startGame(int level, bool newGame);
    void showStartPage(const QString& statusText = QString());

private slots:
    void showBattleMenu();
    void onLevelCleared(int clearedLevel, int nextLevel, bool hasNextLevel);
    void onRunFailed(const QString& message);
};
