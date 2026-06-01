#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QSettings>
#include <QVBoxLayout>

static QPushButton* makeMenuButton(const QString& text) {
    QPushButton* button = new QPushButton(text);
    button->setMinimumSize(320, 66);
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(
        "QPushButton {"
        " background:rgba(255, 255, 255, 235);"
        " color:#22315C;"
        " border:2px solid rgba(130, 150, 220, 220);"
        " border-radius:16px;"
        " font-size:24px;"
        " font-weight:900;"
        " padding:12px 28px;"
        "}"
        "QPushButton:hover {"
        " background:rgba(238, 243, 255, 245);"
        " border:2px solid #FFD36E;"
        "}"
        "QPushButton:pressed {"
        " background:rgba(226, 234, 255, 245);"
        "}"
    );
    return button;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    m_startPage = createStartPage();
    m_gamePage = createGamePage();
    m_transitionPage = createTransitionPage();

    m_stack->addWidget(m_startPage);
    m_stack->addWidget(m_gamePage);
    m_stack->addWidget(m_transitionPage);

    connect(m_controller, &GameController::levelCleared, this, &MainWindow::onLevelCleared);
    connect(m_controller, &GameController::runFailed, this, &MainWindow::onRunFailed);
    connect(m_controller, &GameController::askCloseApplication, this, &MainWindow::close);

    refreshStartMenu();
    m_stack->setCurrentWidget(m_startPage);

    resize(1600, 920);
    setWindowTitle("有伦有类的 RPG - Classifiable Journey");
}

QWidget* MainWindow::createStartPage() {
    QWidget* page = new QWidget;
    page->setObjectName("startPage");
    page->setStyleSheet(
        "#startPage {"
        " background-image:url(:/assets/background_spire.png);"
        " background-position:center;"
        " background-repeat:no-repeat;"
        " background-color:#DCE8FF;"
        "}"
    );

    QFrame* card = new QFrame;
    card->setMaximumWidth(640);
    card->setStyleSheet(
        "QFrame {"
        " background:rgba(255, 255, 255, 215);"
        " border:2px solid rgba(135, 150, 210, 170);"
        " border-radius:28px;"
        "}"
    );

    QLabel* title = new QLabel("CLASSIFIABLE RPG");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#22315C; font-size:50px; font-weight:900; border:0; background:transparent;");

    QLabel* subtitle = new QLabel("阅读源码 · 调用 public 接口 · 穿越类的高塔");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("color:#4E628F; font-size:22px; font-weight:700; border:0; background:transparent;");

    m_startHintLabel = new QLabel;
    m_startHintLabel->setAlignment(Qt::AlignCenter);
    m_startHintLabel->setWordWrap(true);
    m_startHintLabel->setStyleSheet("color:#8A3A00; font-size:20px; font-weight:900; border:0; background:transparent;");

    QPushButton* newButton = makeMenuButton("新的旅程");
    m_continueButton = makeMenuButton("继续你的旅程");
    QPushButton* quitButton = makeMenuButton("退出");

    connect(newButton, &QPushButton::clicked, this, [this]() { startGame(1, true); });
    connect(m_continueButton, &QPushButton::clicked, this, [this]() {
        int saved = savedContinueLevel();
        if (saved < 5) saved = 5;
        startGame(saved, false);
    });
    connect(quitButton, &QPushButton::clicked, this, &MainWindow::close);

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(48, 48, 48, 48);
    cardLayout->setSpacing(18);
    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);
    cardLayout->addWidget(m_startHintLabel);
    cardLayout->addSpacing(18);
    cardLayout->addWidget(m_continueButton, 0, Qt::AlignCenter);
    cardLayout->addWidget(newButton, 0, Qt::AlignCenter);
    cardLayout->addWidget(quitButton, 0, Qt::AlignCenter);

    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addStretch(1);
    pageLayout->addWidget(card, 0, Qt::AlignCenter);
    pageLayout->addStretch(2);

    return page;
}

QWidget* MainWindow::createGamePage() {
    QWidget* central = new QWidget;
    central->setStyleSheet("background:#EDF2FF;");

    m_sceneView = new BattleSceneView;
    m_commandPanel = new CommandPanel;
    m_infoPanel = new InfoCodePanel;
    m_infoPanel->setMinimumWidth(620);

    m_level = new GameLevel(this);
    m_controller = new GameController(this);

    m_menuButton = new QPushButton("菜单");
    m_menuButton->setMinimumHeight(52);
    m_menuButton->setMaximumWidth(140);
    m_menuButton->setCursor(Qt::PointingHandCursor);
    m_menuButton->setStyleSheet(
        "QPushButton {"
        " background:#FFFFFF;"
        " color:#22315C;"
        " border:2px solid #AFC0F5;"
        " border-radius:12px;"
        " font-size:22px;"
        " font-weight:900;"
        " padding:8px 16px;"
        "}"
        "QPushButton:hover { background:#F6F8FF; }"
    );
    connect(m_menuButton, &QPushButton::clicked, this, &MainWindow::showBattleMenu);

    QHBoxLayout* topBar = new QHBoxLayout;
    topBar->setContentsMargins(0, 0, 0, 0);
    topBar->setSpacing(10);
    topBar->addWidget(m_menuButton, 0, Qt::AlignLeft);
    topBar->addStretch(1);

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(10);
    leftLayout->addLayout(topBar, 0);
    leftLayout->addWidget(m_sceneView, 3);
    leftLayout->addWidget(m_commandPanel, 2);

    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(14, 14, 14, 14);
    mainLayout->setSpacing(14);
    mainLayout->addLayout(leftLayout, 3);
    mainLayout->addWidget(m_infoPanel, 2);

    m_controller->setup(m_level, m_sceneView, m_infoPanel, m_commandPanel);
    return central;
}

QWidget* MainWindow::createTransitionPage() {
    QWidget* page = new QWidget;
    page->setObjectName("transitionPage");
    page->setStyleSheet(
        "#transitionPage {"
        " background-image:url(:/assets/background_spire.png);"
        " background-position:center;"
        " background-repeat:no-repeat;"
        " background-color:#DCE8FF;"
        "}"
    );

    QFrame* card = new QFrame;
    card->setMaximumWidth(680);
    card->setStyleSheet(
        "QFrame {"
        " background:rgba(255, 255, 255, 220);"
        " border:2px solid rgba(135, 150, 210, 170);"
        " border-radius:28px;"
        "}"
    );

    m_transitionTitleLabel = new QLabel("关卡完成");
    m_transitionTitleLabel->setAlignment(Qt::AlignCenter);
    m_transitionTitleLabel->setStyleSheet("color:#22315C; font-size:46px; font-weight:900;");

    m_transitionSubtitleLabel = new QLabel("你可以继续前进，也可以退出。");
    m_transitionSubtitleLabel->setAlignment(Qt::AlignCenter);
    m_transitionSubtitleLabel->setWordWrap(true);
    m_transitionSubtitleLabel->setStyleSheet("color:#4E628F; font-size:24px; font-weight:700;");

    m_transitionNextButton = makeMenuButton("下一关");
    m_transitionExitButton = makeMenuButton("退出");

    connect(m_transitionNextButton, &QPushButton::clicked, this, [this]() {
        if (m_pendingNextLevel > GameLevel::TotalLevels) {
            showStartPage("旅程完成。");
            return;
        }
        m_stack->setCurrentWidget(m_gamePage);
        m_controller->startLevel(m_pendingNextLevel);
        m_closeSaveLevel = m_pendingNextLevel;
    });
    connect(m_transitionExitButton, &QPushButton::clicked, this, &MainWindow::close);

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(48, 48, 48, 48);
    cardLayout->setSpacing(18);
    cardLayout->addWidget(m_transitionTitleLabel);
    cardLayout->addWidget(m_transitionSubtitleLabel);
    cardLayout->addSpacing(16);
    cardLayout->addWidget(m_transitionNextButton, 0, Qt::AlignCenter);
    cardLayout->addWidget(m_transitionExitButton, 0, Qt::AlignCenter);

    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addStretch(1);
    pageLayout->addWidget(card, 0, Qt::AlignCenter);
    pageLayout->addStretch(2);

    return page;
}

int MainWindow::savedContinueLevel() const {
    QSettings settings;
    return settings.value("progress/continueLevel", 0).toInt();
}

void MainWindow::refreshStartMenu() {
    const int saved = savedContinueLevel();
    m_continueButton->setVisible(saved >= 5 && saved <= GameLevel::TotalLevels);
    if (saved >= 5) {
        m_continueButton->setText(QString("继续你的旅程  第 %1 关").arg(saved));
    }
}

void MainWindow::startGame(int level, bool newGame) {
    if (newGame) {
        QSettings settings;
        settings.remove("progress/continueLevel");
    }

    m_closeSaveLevel = level;
    m_stack->setCurrentWidget(m_gamePage);
    m_controller->startLevel(level);
}

void MainWindow::showStartPage(const QString& statusText) {
    refreshStartMenu();
    m_startHintLabel->setText(statusText);
    m_stack->setCurrentWidget(m_startPage);
    m_closeSaveLevel = 0;
}

void MainWindow::showBattleMenu() {
    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background:#FFFFFF; border:2px solid #AFC0F5; padding:8px; font-size:18px; font-weight:800; }"
        "QMenu::item { padding:10px 26px; }"
        "QMenu::item:selected { background:#EDF2FF; }"
    );

    QAction* exitAction = menu.addAction("退出游戏");
    QAction* retryAction = menu.addAction("重玩本关");
    QAction* hintAction = menu.addAction("获得提示");

    QAction* chosen = menu.exec(m_menuButton->mapToGlobal(QPoint(0, m_menuButton->height())));
    if (chosen == exitAction) {
        close();
    } else if (chosen == retryAction) {
        m_controller->restartCurrentLevel();
    } else if (chosen == hintAction) {
        m_controller->showHint();
    }
}

void MainWindow::onLevelCleared(int clearedLevel, int nextLevel, bool hasNextLevel) {
    m_pendingNextLevel = nextLevel;
    m_closeSaveLevel = hasNextLevel ? nextLevel : 0;

    if (hasNextLevel) {
        m_transitionTitleLabel->setText(QString("第 %1 关完成").arg(clearedLevel));
        m_transitionSubtitleLabel->setText(QString("准备进入第 %1 关。").arg(nextLevel));
        m_transitionNextButton->setText("下一关");
        m_transitionNextButton->setVisible(true);
    } else {
        m_transitionTitleLabel->setText("旅程完成");
        m_transitionSubtitleLabel->setText("你已经完成全部关卡。");
        m_transitionNextButton->setText("返回开始");
        m_transitionNextButton->setVisible(true);
    }

    m_stack->setCurrentWidget(m_transitionPage);
}

void MainWindow::onRunFailed(const QString& message) {
    QSettings settings;
    settings.remove("progress/continueLevel");
    showStartPage(message);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    QSettings settings;
    if (m_closeSaveLevel >= 5 && m_closeSaveLevel <= GameLevel::TotalLevels) {
        settings.setValue("progress/continueLevel", m_closeSaveLevel);
    }
    QMainWindow::closeEvent(event);
}
