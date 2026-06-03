#include "MainWindow.h"

#include <cmath>

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QDialog>
#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QSettings>
#include <QStringList>
#include <QtGlobal>
#include <QTimer>
#include <QVBoxLayout>


class ParameterArrowOverlay : public QWidget {
private:
    bool m_active = false;
    QPoint m_startGlobal;
    QTimer* m_timer = nullptr;

public:
    explicit ParameterArrowOverlay(QWidget* parent = nullptr)
        : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAutoFillBackground(false);
        if (parent) {
            setGeometry(parent->rect());
            parent->installEventFilter(this);
        }
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, [this]() {
            if (m_active) update();
        });
        m_timer->start(33);
        hide();
    }

    void setArrow(bool active, const QPoint& startGlobal = QPoint()) {
        m_active = active;
        m_startGlobal = startGlobal;
        if (parentWidget()) setGeometry(parentWidget()->rect());
        if (m_active) {
            show();
            raise();
        } else {
            hide();
        }
        update();
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (watched == parentWidget() && event->type() == QEvent::Resize) {
            setGeometry(parentWidget()->rect());
            raise();
        }
        return QWidget::eventFilter(watched, event);
    }

    void paintEvent(QPaintEvent*) override {
        if (!m_active) return;

        const QPointF start = mapFromGlobal(m_startGlobal);
        const QPointF end = mapFromGlobal(QCursor::pos());
        QLineF line(start, end);
        const qreal length = line.length();
        if (length < 18.0) return;

        const qreal dx = line.dx() / length;
        const qreal dy = line.dy() / length;
        const QPointF dir(dx, dy);
        const QPointF normal(-dy, dx);
        const qreal headLength = qBound<qreal>(28.0, length * 0.16, 48.0);
        const qreal headHalfWidth = qBound<qreal>(16.0, length * 0.075, 25.0);
        const QPointF bodyEnd = end - QPointF(dir.x() * headLength * 0.82, dir.y() * headLength * 0.82);
        const QPointF headBase = end - QPointF(dir.x() * headLength, dir.y() * headLength);

        QPolygonF head;
        head << end
             << headBase + QPointF(normal.x() * headHalfWidth, normal.y() * headHalfWidth)
             << headBase - QPointF(normal.x() * headHalfWidth, normal.y() * headHalfWidth);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor(36, 80, 180, 80), 16.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(start, bodyEnd);
        painter.setPen(QPen(QColor(255, 255, 255, 170), 10.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(start, bodyEnd);
        painter.setPen(QPen(QColor(63, 122, 255, 190), 6.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(start, bodyEnd);
        painter.setPen(QPen(QColor(255, 255, 255, 175), 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(QColor(74, 132, 255, 185));
        painter.drawPolygon(head);
        painter.setPen(QPen(QColor(63, 122, 255, 185), 4.0));
        painter.setBrush(QColor(255, 255, 255, 120));
        painter.drawEllipse(start, 7.0, 7.0);
    }
};

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
        "QPushButton:disabled {"
        " background:rgba(210, 215, 230, 210);"
        " color:#8A93A8;"
        " border:2px solid rgba(150, 155, 170, 160);"
        "}"
    );
    return button;
}

static QFrame* makePopupCard(const QString& titleText, const QString& subtitleText, QVBoxLayout** buttonLayoutOut) {
    QFrame* card = new QFrame;
    card->setMinimumWidth(500);
    card->setMaximumWidth(620);
    card->setStyleSheet(
        "QFrame {"
        " background:rgba(255, 255, 255, 242);"
        " border:2px solid rgba(135, 150, 210, 190);"
        " border-radius:26px;"
        "}"
    );

    QLabel* title = new QLabel(titleText);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#22315C; font-size:38px; font-weight:900; border:0; background:transparent;");

    QLabel* subtitle = new QLabel(subtitleText);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setVisible(!subtitleText.isEmpty());
    subtitle->setStyleSheet("color:#4E628F; font-size:28px; font-weight:900; border:0; background:transparent;");

    QVBoxLayout* buttons = new QVBoxLayout;
    buttons->setSpacing(12);

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(40, 38, 40, 38);
    cardLayout->setSpacing(16);
    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(8);
    cardLayout->addLayout(buttons);

    if (buttonLayoutOut) {
        *buttonLayoutOut = buttons;
    }
    return card;
}

static void preparePopupDialog(QDialog& dialog, QWidget* parent, const QString& title, const QString& subtitle, QVBoxLayout** buttonLayoutOut) {
    dialog.setModal(true);
    dialog.setWindowTitle(title);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dialog.setStyleSheet("QDialog { background:#EEF3FF; }");

    QVBoxLayout* root = new QVBoxLayout(&dialog);
    root->setContentsMargins(18, 18, 18, 18);
    root->addWidget(makePopupCard(title, subtitle, buttonLayoutOut), 0, Qt::AlignCenter);
}

static void centerPopupDialog(QDialog& dialog, QWidget* parent) {
    dialog.adjustSize();
    if (parent) {
        const QPoint center = parent->geometry().center();
        dialog.move(center - QPoint(dialog.width() / 2, dialog.height() / 2));
    }
}

static QString unlockedLevelButtonStyle() {
    return QString(
        "QPushButton {"
        " background:rgba(255, 255, 255, 238);"
        " color:#22315C;"
        " border:2px solid rgba(130, 150, 220, 220);"
        " border-radius:18px;"
        " font-size:18px;"
        " font-weight:900;"
        " padding:10px 14px;"
        "}"
        "QPushButton:hover {"
        " background:rgba(238, 243, 255, 248);"
        " border:2px solid #FFD36E;"
        "}"
    );
}

static QString lockedLevelButtonStyle() {
    return QString(
        "QPushButton {"
        " background:rgba(205, 211, 225, 215);"
        " color:#7B8496;"
        " border:2px solid rgba(145, 150, 166, 170);"
        " border-radius:18px;"
        " font-size:18px;"
        " font-weight:900;"
        " padding:10px 14px;"
        "}"
    );
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    qApp->installEventFilter(this);
    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    m_startPage = createStartPage();
    m_levelSelectPage = createLevelSelectPage();
    m_gamePage = createGamePage();
    m_transitionPage = createTransitionPage();

    m_stack->addWidget(m_startPage);
    m_stack->addWidget(m_levelSelectPage);
    m_stack->addWidget(m_gamePage);
    m_stack->addWidget(m_transitionPage);

    connect(m_controller, &GameController::levelCleared, this, &MainWindow::onLevelCleared);
    connect(m_controller, &GameController::runFailed, this, &MainWindow::onRunFailed);
    connect(m_controller, &GameController::askCloseApplication, this, &MainWindow::close);

    refreshStartMenu();
    refreshLevelSelectPage();
    showStartPage();

    resize(1600, 920);
    setWindowTitle("有伦有类的 RPG - Classifiable Journey");
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (event && event->type() == QEvent::MouseButtonPress && m_commandPanel && m_commandPanel->isParameterSelectionActive()) {
        QWidget* widget = qobject_cast<QWidget*>(watched);
        if (widget) {
            const bool inCommandPanel = widget == m_commandPanel || m_commandPanel->isAncestorOf(widget);
            const bool inSceneView = m_sceneView && (widget == m_sceneView || m_sceneView->isAncestorOf(widget));
            if (!inCommandPanel && !inSceneView) {
                m_commandPanel->cancelParameterSelection();
                if (m_sceneView) m_sceneView->setParameterPickMode(false);
                if (m_parameterArrowOverlay) m_parameterArrowOverlay->setArrow(false);
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
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
    card->setMaximumWidth(680);
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

    QPushButton* selectButton = makeMenuButton("选择关卡");
    m_continueButton = makeMenuButton("继续挑战");
    QPushButton* quitButton = makeMenuButton("退出");

    connect(selectButton, &QPushButton::clicked, this, [this]() { showLevelSelectPage(); });
    connect(m_continueButton, &QPushButton::clicked, this, [this]() { startGame(highestUnlockedLevel()); });
    connect(quitButton, &QPushButton::clicked, this, &MainWindow::close);

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(48, 48, 48, 48);
    cardLayout->setSpacing(18);
    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);
    cardLayout->addWidget(m_startHintLabel);
    cardLayout->addSpacing(18);
    cardLayout->addWidget(selectButton, 0, Qt::AlignCenter);
    cardLayout->addWidget(m_continueButton, 0, Qt::AlignCenter);
    cardLayout->addWidget(quitButton, 0, Qt::AlignCenter);

    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addStretch(1);
    pageLayout->addWidget(card, 0, Qt::AlignCenter);
    pageLayout->addStretch(2);

    return page;
}

QWidget* MainWindow::createLevelSelectPage() {
    QWidget* page = new QWidget;
    page->setObjectName("levelSelectPage");
    page->setStyleSheet(
        "#levelSelectPage {"
        " background-image:url(:/assets/background_spire.png);"
        " background-position:center;"
        " background-repeat:no-repeat;"
        " background-color:#DCE8FF;"
        "}"
    );

    QFrame* card = new QFrame;
    card->setMaximumWidth(840);
    card->setStyleSheet(
        "QFrame {"
        " background:rgba(255, 255, 255, 220);"
        " border:2px solid rgba(135, 150, 210, 170);"
        " border-radius:28px;"
        "}"
    );

    QLabel* title = new QLabel("选择关卡");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#22315C; font-size:46px; font-weight:900; border:0; background:transparent;");

    m_levelSelectHintLabel = new QLabel;
    m_levelSelectHintLabel->setAlignment(Qt::AlignCenter);
    m_levelSelectHintLabel->setWordWrap(true);
    m_levelSelectHintLabel->setStyleSheet("color:#4E628F; font-size:20px; font-weight:800; border:0; background:transparent;");

    QGridLayout* grid = new QGridLayout;
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(16);
    m_levelButtons.clear();

    for (int i = 1; i <= GameLevel::TotalLevels; ++i) {
        QPushButton* button = new QPushButton;
        button->setMinimumSize(154, 104);
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, [this, i]() {
            if (isLevelUnlocked(i)) {
                startGame(i);
            }
        });
        m_levelButtons.push_back(button);
        grid->addWidget(button, (i - 1) / 5, (i - 1) % 5);
    }

    QPushButton* backButton = makeMenuButton("返回主界面");
    connect(backButton, &QPushButton::clicked, this, [this]() { showStartPage(); });

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(44, 44, 44, 44);
    cardLayout->setSpacing(18);
    cardLayout->addWidget(title);
    cardLayout->addWidget(m_levelSelectHintLabel);
    cardLayout->addSpacing(10);
    cardLayout->addLayout(grid);
    cardLayout->addSpacing(12);
    cardLayout->addWidget(backButton, 0, Qt::AlignCenter);

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
    m_infoPanel->setMinimumWidth(600);

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

    m_levelTitleLabel = new QLabel;
    m_levelTitleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_levelTitleLabel->setStyleSheet(
        "color:#22315C;"
        "font-size:24px;"
        "font-weight:900;"
        "background:transparent;"
        "border:0;"
        "padding:0 0 0 12px;"
    );

    QHBoxLayout* topBar = new QHBoxLayout;
    topBar->setContentsMargins(0, 0, 0, 0);
    topBar->setSpacing(8);
    topBar->addWidget(m_menuButton, 0, Qt::AlignLeft);
    topBar->addWidget(m_levelTitleLabel, 1);
    topBar->addStretch(1);

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(8);
    leftLayout->addLayout(topBar, 0);
    leftLayout->addWidget(m_sceneView, 3);
    leftLayout->addWidget(m_commandPanel, 2);

    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    mainLayout->addLayout(leftLayout, 3);
    mainLayout->addWidget(m_infoPanel, 2);

    m_controller->setup(m_level, m_sceneView, m_infoPanel, m_commandPanel);

    m_parameterArrowOverlay = new ParameterArrowOverlay(central);
    connect(m_commandPanel, &CommandPanel::parameterSelectionChanged, this, [this](bool active, const QPoint& startGlobal) {
        if (m_parameterArrowOverlay) m_parameterArrowOverlay->setArrow(active, startGlobal);
    });

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
    card->setMaximumWidth(720);
    card->setStyleSheet(
        "QFrame {"
        " background:rgba(255, 255, 255, 220);"
        " border:2px solid rgba(135, 150, 210, 170);"
        " border-radius:28px;"
        "}"
    );

    m_transitionTitleLabel = new QLabel("关卡结束");
    m_transitionTitleLabel->setAlignment(Qt::AlignCenter);
    m_transitionTitleLabel->setStyleSheet("color:#22315C; font-size:46px; font-weight:900;");

    m_transitionSubtitleLabel = new QLabel("✦");
    m_transitionSubtitleLabel->setAlignment(Qt::AlignCenter);
    m_transitionSubtitleLabel->setWordWrap(true);
    m_transitionSubtitleLabel->setStyleSheet("color:#4E628F; font-size:24px; font-weight:700;");

    m_transitionRetryButton = makeMenuButton("重新游玩本关");
    m_transitionNextButton = makeMenuButton("进入下一关");
    m_transitionMainButton = makeMenuButton("返回主界面");
    m_transitionSelectButton = makeMenuButton("返回选关界面");

    connect(m_transitionRetryButton, &QPushButton::clicked, this, [this]() {
        startGame(m_transitionLevel);
    });
    connect(m_transitionNextButton, &QPushButton::clicked, this, [this]() {
        if (m_pendingNextLevel >= 1 &&
            m_pendingNextLevel <= GameLevel::TotalLevels &&
            isLevelUnlocked(m_pendingNextLevel)) {
            startGame(m_pendingNextLevel);
        }
    });
    connect(m_transitionMainButton, &QPushButton::clicked, this, [this]() {
        showStartPage();
    });
    connect(m_transitionSelectButton, &QPushButton::clicked, this, [this]() {
        showLevelSelectPage();
    });

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(48, 48, 48, 48);
    cardLayout->setSpacing(18);
    cardLayout->addWidget(m_transitionTitleLabel);
    cardLayout->addWidget(m_transitionSubtitleLabel);
    cardLayout->addSpacing(16);
    cardLayout->addWidget(m_transitionRetryButton, 0, Qt::AlignCenter);
    cardLayout->addWidget(m_transitionNextButton, 0, Qt::AlignCenter);
    cardLayout->addWidget(m_transitionMainButton, 0, Qt::AlignCenter);
    cardLayout->addWidget(m_transitionSelectButton, 0, Qt::AlignCenter);

    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addStretch(1);
    pageLayout->addWidget(card, 0, Qt::AlignCenter);
    pageLayout->addStretch(2);

    return page;
}

int MainWindow::highestUnlockedLevel() const {
    QSettings settings;
    return qBound(1, settings.value("progress/highestUnlockedLevel_v2", 1).toInt(), GameLevel::TotalLevels);
}

void MainWindow::setHighestUnlockedLevel(int level) const {
    QSettings settings;
    settings.setValue("progress/highestUnlockedLevel_v2", qBound(1, level, GameLevel::TotalLevels));
}

bool MainWindow::isLevelUnlocked(int level) const {
    return level >= 1 && level <= highestUnlockedLevel();
}

void MainWindow::refreshStartMenu() {
    const int highest = highestUnlockedLevel();
    m_continueButton->setText(QString("继续挑战  第 %1 关 · %2").arg(highest).arg(GameLevel::levelName(highest)));
}

void MainWindow::refreshLevelSelectPage() {
    const int highest = highestUnlockedLevel();
    if (m_levelSelectHintLabel) {
        QStringList progressMarks;
        for (int level = 1; level <= GameLevel::TotalLevels; ++level) {
            progressMarks << (level <= highest ? QStringLiteral("●") : QStringLiteral("○"));
        }
        m_levelSelectHintLabel->setText(progressMarks.join(QStringLiteral("   ")));
    }

    for (int i = 0; i < m_levelButtons.size(); ++i) {
        const int level = i + 1;
        QPushButton* button = m_levelButtons[i];
        const bool unlocked = level <= highest;
        button->setEnabled(unlocked);
        button->setCursor(unlocked ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
        button->setStyleSheet(unlocked ? unlockedLevelButtonStyle() : lockedLevelButtonStyle());

        const QString name = GameLevel::levelName(level);
        if (!unlocked) {
            button->setText(QString("第 %1 关\n%2\n🔒").arg(level).arg(name));
        } else if (level < highest) {
            button->setText(QString("第 %1 关\n%2\n✓").arg(level).arg(name));
        } else {
            button->setText(QString("第 %1 关\n%2\n▶").arg(level).arg(name));
        }
    }
}

void MainWindow::startGame(int level) {
    if (!isLevelUnlocked(level)) {
        showLevelSelectPage();
        return;
    }

    if (m_levelTitleLabel) {
        m_levelTitleLabel->setText(QString("第 %1 关 · %2").arg(level).arg(GameLevel::levelName(level)));
    }
    m_stack->setCurrentWidget(m_gamePage);
    m_controller->startLevel(level);
}

void MainWindow::showStartPage(const QString& statusText) {
    refreshStartMenu();
    m_startHintLabel->setText(statusText);
    m_startHintLabel->setVisible(!statusText.isEmpty());
    m_stack->setCurrentWidget(m_startPage);
}

void MainWindow::showLevelSelectPage(const QString& statusText) {
    refreshLevelSelectPage();
    if (!statusText.isEmpty() && m_levelSelectHintLabel) {
        m_levelSelectHintLabel->setText(statusText);
    }
    m_stack->setCurrentWidget(m_levelSelectPage);
}

void MainWindow::showBattleMenu() {
    QDialog dialog(this);
    QVBoxLayout* buttons = nullptr;
    preparePopupDialog(dialog, this, QStringLiteral("菜单"), QString(), &buttons);

    QPushButton* retryButton = makeMenuButton(QStringLiteral("重玩本关"));
    QPushButton* mainButton = makeMenuButton(QStringLiteral("返回主界面"));
    QPushButton* selectButton = makeMenuButton(QStringLiteral("返回选关界面"));
    QPushButton* hintButton = makeMenuButton(QStringLiteral("提示"));

    buttons->addWidget(retryButton, 0, Qt::AlignCenter);
    buttons->addWidget(mainButton, 0, Qt::AlignCenter);
    buttons->addWidget(selectButton, 0, Qt::AlignCenter);
    buttons->addWidget(hintButton, 0, Qt::AlignCenter);

    connect(retryButton, &QPushButton::clicked, this, [this, &dialog]() {
        dialog.accept();
        if (m_controller) m_controller->restartCurrentLevel();
    });
    connect(mainButton, &QPushButton::clicked, this, [this, &dialog]() {
        dialog.accept();
        showStartPage();
    });
    connect(selectButton, &QPushButton::clicked, this, [this, &dialog]() {
        dialog.accept();
        showLevelSelectPage();
    });
    connect(hintButton, &QPushButton::clicked, this, [this, &dialog]() {
        dialog.accept();
        if (m_controller) m_controller->showHint();
    });

    centerPopupDialog(dialog, this);
    dialog.exec();
}

void MainWindow::showLevelResultDialog(bool cleared, int level, int nextLevel, bool canEnterNextLevel) {
    QDialog dialog(this);
    QVBoxLayout* buttons = nullptr;
    const QString title = cleared
        ? QStringLiteral("第 %1 关完成").arg(level)
        : QStringLiteral("第 %1 关失败").arg(level);
    const QString subtitle = cleared ? QStringLiteral("✓") : QStringLiteral("×");
    preparePopupDialog(dialog, this, title, subtitle, &buttons);

    QPushButton* retryButton = makeMenuButton(QStringLiteral("重新游玩本关"));
    QPushButton* nextButton = makeMenuButton(canEnterNextLevel
        ? QStringLiteral("进入第 %1 关").arg(nextLevel)
        : QStringLiteral("进入下一关"));
    QPushButton* mainButton = makeMenuButton(QStringLiteral("返回主界面"));
    QPushButton* selectButton = makeMenuButton(QStringLiteral("返回选关界面"));

    nextButton->setEnabled(canEnterNextLevel);

    buttons->addWidget(retryButton, 0, Qt::AlignCenter);
    buttons->addWidget(nextButton, 0, Qt::AlignCenter);
    buttons->addWidget(mainButton, 0, Qt::AlignCenter);
    buttons->addWidget(selectButton, 0, Qt::AlignCenter);

    connect(retryButton, &QPushButton::clicked, this, [this, &dialog, level]() {
        dialog.accept();
        startGame(level);
    });
    connect(nextButton, &QPushButton::clicked, this, [this, &dialog, nextLevel]() {
        dialog.accept();
        startGame(nextLevel);
    });
    connect(mainButton, &QPushButton::clicked, this, [this, &dialog]() {
        dialog.accept();
        showStartPage();
    });
    connect(selectButton, &QPushButton::clicked, this, [this, &dialog]() {
        dialog.accept();
        showLevelSelectPage();
    });

    centerPopupDialog(dialog, this);
    dialog.exec();
}

void MainWindow::onLevelCleared(int clearedLevel, int nextLevel, bool hasNextLevel) {
    m_transitionLevel = clearedLevel;
    m_pendingNextLevel = nextLevel;

    if (hasNextLevel) {
        setHighestUnlockedLevel(qMax(highestUnlockedLevel(), nextLevel));
    } else {
        setHighestUnlockedLevel(GameLevel::TotalLevels);
    }
    refreshStartMenu();
    refreshLevelSelectPage();

    const bool canEnterNextLevel = hasNextLevel && isLevelUnlocked(nextLevel);
    showLevelResultDialog(true, clearedLevel, nextLevel, canEnterNextLevel);
}

void MainWindow::onRunFailed(const QString& message) {
    Q_UNUSED(message);
    const int failedLevel = m_controller ? m_controller->currentLevel() : 1;
    const int nextLevel = failedLevel + 1;

    m_transitionLevel = failedLevel;
    m_pendingNextLevel = nextLevel;

    refreshStartMenu();
    refreshLevelSelectPage();

    const bool canEnterNextLevel = nextLevel <= GameLevel::TotalLevels && isLevelUnlocked(nextLevel);
    showLevelResultDialog(false, failedLevel, nextLevel, canEnterNextLevel);
}
