#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QPushButton>
#include <QStackedWidget>
#include <QVector>

#include "BattleSceneView.h"
#include "CommandPanel.h"
#include "GameController.h"
#include "GameLevel.h"
#include "InfoCodePanel.h"

class ParameterArrowOverlay;

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    QStackedWidget* m_stack = nullptr;
    QWidget* m_startPage = nullptr;
    QWidget* m_levelSelectPage = nullptr;
    QWidget* m_gamePage = nullptr;
    QWidget* m_transitionPage = nullptr;

    QLabel* m_startHintLabel = nullptr;
    QLabel* m_levelSelectHintLabel = nullptr;
    QPushButton* m_continueButton = nullptr;
    QVector<QPushButton*> m_levelButtons;

    QLabel* m_transitionTitleLabel = nullptr;
    QLabel* m_transitionSubtitleLabel = nullptr;
    QPushButton* m_transitionRetryButton = nullptr;
    QPushButton* m_transitionNextButton = nullptr;
    QPushButton* m_transitionMainButton = nullptr;
    QPushButton* m_transitionSelectButton = nullptr;

    QPushButton* m_menuButton = nullptr;
    QLabel* m_levelTitleLabel = nullptr;

    BattleSceneView* m_sceneView = nullptr;
    CommandPanel* m_commandPanel = nullptr;
    InfoCodePanel* m_infoPanel = nullptr;
    ParameterArrowOverlay* m_parameterArrowOverlay = nullptr;
    GameLevel* m_level = nullptr;
    GameController* m_controller = nullptr;

    int m_pendingNextLevel = 1;
    int m_transitionLevel = 1;

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QWidget* createStartPage();
    QWidget* createLevelSelectPage();
    QWidget* createGamePage();
    QWidget* createTransitionPage();

    int highestUnlockedLevel() const;
    void setHighestUnlockedLevel(int level) const;
    bool isLevelUnlocked(int level) const;
    void refreshStartMenu();
    void refreshLevelSelectPage();
    void startGame(int level);
    void showStartPage(const QString& statusText = QString());
    void showLevelSelectPage(const QString& statusText = QString());
    void showLevelResultDialog(bool cleared, int level, int nextLevel, bool canEnterNextLevel);

private slots:
    void showBattleMenu();
    void onLevelCleared(int clearedLevel, int nextLevel, bool hasNextLevel);
    void onRunFailed(const QString& message);
};
