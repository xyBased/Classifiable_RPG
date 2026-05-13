#include "MainWindow.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    m_sceneView = new BattleSceneView;
    m_commandPanel = new CommandPanel;
    m_infoPanel = new InfoCodePanel;

    m_infoPanel->setMinimumWidth(520);

    m_level = new GameLevel(this);
    m_controller = new GameController(this);

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->addWidget(m_sceneView, 3);
    leftLayout->addWidget(m_commandPanel, 2);

    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(12);

    mainLayout->addLayout(leftLayout, 3);
    mainLayout->addWidget(m_infoPanel, 2);

    m_controller->setup(
        m_level,
        m_sceneView,
        m_infoPanel,
        m_commandPanel
        );

    m_controller->startLevel1();

    resize(1280, 720);

    setWindowTitle("有伦有类的 RPG - 策略解谜版");
}

