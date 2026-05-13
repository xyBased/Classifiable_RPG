#include "InfoCodePanel.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QUrl>
#include <QTextOption>
#include <QScrollBar>

static QLabel* makeNameLabel(const QString& text) {
    QLabel* label = new QLabel(text);
    label->setStyleSheet(
        "color: #8BD5FF;"
        "font-weight: 700;"
        "font-size: 15px;"
        );
    return label;
}

static QLabel* makeValueLabel() {
    QLabel* label = new QLabel("-");
    label->setStyleSheet(
        "color: #FFFFFF;"
        "font-size: 15px;"
        );
    return label;
}

InfoCodePanel::InfoCodePanel(QWidget* parent)
    : QWidget(parent) {

    m_titleLabel = new QLabel("当前选中： 未选中");
    m_propertyLabel = new QLabel("属性");
    m_codeLabel = new QLabel("类代码");

    m_propertyCard = new QFrame;
    m_hpBar = new QProgressBar;

    m_idValueLabel = makeValueLabel();
    m_classValueLabel = makeValueLabel();
    m_atkValueLabel = makeValueLabel();
    m_campValueLabel = makeValueLabel();

    m_codeView = new QTextBrowser;

    // ===== 标题样式 =====
    m_titleLabel->setStyleSheet(
        "color: #FFFFFF;"
        "font-size: 22px;"
        "font-weight: 800;"
        "padding: 4px 2px;"
        );

    m_propertyLabel->setStyleSheet(
        "color: #8BD5FF;"
        "font-size: 15px;"
        "font-weight: 800;"
        "padding-top: 6px;"
        );

    m_codeLabel->setStyleSheet(
        "color: #FFD36E;"
        "font-size: 15px;"
        "font-weight: 800;"
        "padding-top: 8px;"
        );

    // ===== 属性卡片 =====
    m_propertyCard->setStyleSheet(
        "QFrame {"
        "   background: #252632;"
        "   border: 1px solid #59607A;"
        "   border-radius: 10px;"
        "}"
        );

    QLabel* hpTitle = makeNameLabel("生命值");

    m_hpBar->setRange(0, 100);
    m_hpBar->setValue(100);
    m_hpBar->setTextVisible(true);
    m_hpBar->setFormat("- / -");
    m_hpBar->setFixedHeight(22);

    m_hpBar->setStyleSheet(
        "QProgressBar {"
        "   background-color: #1D2130;"
        "   border: 1px solid #8FA2D6;"
        "   border-radius: 3px;"
        "   text-align: center;"
        "   color: #FFFFFF;"
        "   font-size: 14px;"
        "   font-weight: 800;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: #6EDB74;"
        "   border-radius: 2px;"
        "}"
        );


    QGridLayout* infoGrid = new QGridLayout;
    infoGrid->setHorizontalSpacing(12);
    infoGrid->setVerticalSpacing(7);
    infoGrid->setColumnMinimumWidth(0, 82);
    infoGrid->setColumnStretch(0, 0);
    infoGrid->setColumnStretch(1, 1);

    // 第 0 行：生命值 + 血条，放在同一行
    infoGrid->addWidget(hpTitle, 0, 0);
    infoGrid->addWidget(m_hpBar, 0, 1);

    // 下面是普通属性
    infoGrid->addWidget(makeNameLabel("对象名"), 1, 0);
    infoGrid->addWidget(m_idValueLabel, 1, 1);

    infoGrid->addWidget(makeNameLabel("类名"), 2, 0);
    infoGrid->addWidget(m_classValueLabel, 2, 1);

    infoGrid->addWidget(makeNameLabel("攻击力"), 3, 0);
    infoGrid->addWidget(m_atkValueLabel, 3, 1);

    infoGrid->addWidget(makeNameLabel("阵营"), 4, 0);
    infoGrid->addWidget(m_campValueLabel, 4, 1);

    QVBoxLayout* cardLayout = new QVBoxLayout(m_propertyCard);
    cardLayout->setContentsMargins(16, 14, 16, 14);
    cardLayout->setSpacing(6);
    cardLayout->addLayout(infoGrid);
    cardLayout->addStretch();

    // ===== 代码区 =====
    m_codeView->setOpenLinks(false);
    m_codeView->setOpenExternalLinks(false);

    m_codeView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_codeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QTextOption option = m_codeView->document()->defaultTextOption();
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_codeView->document()->setDefaultTextOption(option);

    m_codeView->setStyleSheet(
        "QTextBrowser {"
        "   background: #1E2230;"
        "   color: #EAF1FF;"
        "   border: 1px solid #59607A;"
        "   border-radius: 10px;"
        "   padding: 12px;"
        "   selection-background-color: #465BFF;"
        "   font-family: Consolas, 'Courier New', monospace;"
        "   font-size: 13px;"
        "}"
        "QTextBrowser a {"
        "   color: #89DDFF;"
        "   text-decoration: underline;"
        "   font-weight: 700;"
        "}"
        );

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_propertyLabel);
    layout->addWidget(m_propertyCard, 0);
    layout->addWidget(m_codeLabel);
    layout->addWidget(m_codeView, 1);

    connect(m_codeView, &QTextBrowser::anchorClicked,
            this, &InfoCodePanel::handleAnchorClicked);

    showEmpty();
}

void InfoCodePanel::showCreature(Creature* creature) {
    if (!creature) {
        showEmpty();
        return;
    }

    m_titleLabel->setText("当前选中： " + creature->id());

    int hp = creature->hp();
    int maxHp = creature->maxHp();

    int percent = 0;
    if (maxHp > 0) {
        percent = static_cast<int>(100.0 * hp / maxHp);
    }

    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    QString hpColor = "#6EDB74";   // 绿
    if (percent <= 30) {
        hpColor = "#FF6B6B";       // 红
    } else if (percent <= 60) {
        hpColor = "#FFD36E";       // 黄
    }

    m_hpBar->setValue(percent);
    m_hpBar->setFormat(QString("%1 / %2").arg(hp).arg(maxHp));

    m_hpBar->setStyleSheet(
        QString(
            "QProgressBar {"
            "   background-color: #1D2130;"
            "   border: 1px solid #8FA2D6;"
            "   border-radius: 3px;"
            "   text-align: center;"
            "   color: #FFFFFF;"
            "   font-size: 14px;"
            "   font-weight: 800;"
            "}"
            "QProgressBar::chunk {"
            "   background-color: %1;"
            "   border-radius: 2px;"
            "}"
            ).arg(hpColor)
        );


    m_idValueLabel->setText(creature->id());
    m_classValueLabel->setText(creature->className());
    m_atkValueLabel->setText(QString::number(creature->atk()));
    m_campValueLabel->setText(creature->isEnemy() ? "Enemy" : "Player");

    m_classValueLabel->setStyleSheet(
        "color: #FFD36E;"
        "font-size: 15px;"
        "font-weight: 700;"
        );

    m_codeView->setHtml(creature->classCodeHtml());
    m_codeView->verticalScrollBar()->setValue(0);
}

void InfoCodePanel::showEmpty() {
    m_titleLabel->setText("当前选中： 未选中");

    m_hpBar->setValue(0);
    m_hpBar->setFormat("- / -");

    m_idValueLabel->setText("-");
    m_classValueLabel->setText("-");
    m_atkValueLabel->setText("-");
    m_campValueLabel->setText("-");

    m_codeView->setHtml(
        "<div style='font-size:14px; color:#C9D1D9; line-height:1.5;'>"
        "这里会显示该对象的类代码。"
        "</div>"
        );
}


void InfoCodePanel::handleAnchorClicked(const QUrl& url) {
    QString command = url.toString();

    if (command.startsWith("cmd:")) {
        command.remove(0, 4);
    }

    emit commandChosen(command);
}
