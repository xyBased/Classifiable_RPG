#include "InfoCodePanel.h"

#include <QGridLayout>
#include <QScrollBar>
#include <QTextOption>
#include <QVBoxLayout>

static QLabel* makeSmallTitle(const QString& text) {
    QLabel* label = new QLabel(text);
    label->setStyleSheet("color:#5872C9; font-weight:900; font-size:18px;");
    return label;
}

InfoCodePanel::InfoCodePanel(QWidget* parent)
    : QWidget(parent) {
    setStyleSheet("QWidget { background:#EDF2FF; }");

    m_titleLabel = new QLabel("未选中");
    m_subtitleLabel = new QLabel("点击对象查看实现");
    m_propertyLabel = new QLabel("状态");
    m_codeLabel = new QLabel("源码");
    m_statCard = new QFrame;
    m_hpBar = new QProgressBar;
    m_atkLabel = new QLabel("ATK -");
    m_campLabel = new QLabel("-");
    m_codeView = new QTextBrowser;

    m_titleLabel->setStyleSheet("color:#22315C; font-size:30px; font-weight:900; padding:2px;");
    m_subtitleLabel->setStyleSheet("color:#51638F; font-size:18px; font-weight:700; padding-left:2px;");
    m_propertyLabel->setStyleSheet("color:#3656B2; font-size:19px; font-weight:900; padding-top:8px;");
    m_codeLabel->setStyleSheet("color:#7B4C00; font-size:19px; font-weight:900; padding-top:8px;");

    m_statCard->setStyleSheet(
        "QFrame {"
        " background:#FFFFFF;"
        " border:2px solid #AFC0F5;"
        " border-radius:16px;"
        "}"
    );
    m_statCard->setFixedHeight(140);

    m_hpBar->setRange(0, 100);
    m_hpBar->setFixedHeight(28);
    m_hpBar->setTextVisible(true);
    m_hpBar->setFormat("- / -");
    m_hpBar->setStyleSheet(
        "QProgressBar {"
        " background-color:#EEF3FF;"
        " border:2px solid #AFC0F5;"
        " border-radius:9px;"
        " text-align:center;"
        " color:#22315C;"
        " font-weight:900;"
        " font-size:16px;"
        "}"
        "QProgressBar::chunk {"
        " background-color:#6EDB74;"
        " border-radius:7px;"
        "}"
    );

    m_atkLabel->setStyleSheet("color:#22315C; font-size:20px; font-weight:900;");
    m_campLabel->setStyleSheet("color:#7B4C00; font-size:20px; font-weight:900;");

    QGridLayout* statGrid = new QGridLayout;
    statGrid->setHorizontalSpacing(14);
    statGrid->setVerticalSpacing(10);
    statGrid->addWidget(makeSmallTitle("HP"), 0, 0);
    statGrid->addWidget(m_hpBar, 0, 1);
    statGrid->addWidget(makeSmallTitle("ATK"), 1, 0);
    statGrid->addWidget(m_atkLabel, 1, 1);
    statGrid->addWidget(makeSmallTitle("阵营"), 2, 0);
    statGrid->addWidget(m_campLabel, 2, 1);

    QVBoxLayout* cardLayout = new QVBoxLayout(m_statCard);
    cardLayout->setContentsMargins(18, 12, 18, 12);
    cardLayout->addLayout(statGrid);

    m_codeView->setOpenLinks(false);
    m_codeView->setOpenExternalLinks(false);
    m_codeView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_codeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QTextOption option = m_codeView->document()->defaultTextOption();
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_codeView->document()->setDefaultTextOption(option);

    m_codeView->setStyleSheet(
        "QTextBrowser {"
        " background:#FFFFFF;"
        " color:#22315C;"
        " border:2px solid #AFC0F5;"
        " border-radius:16px;"
        " padding:14px;"
        " selection-background-color:#5E72FF;"
        " font-family:Consolas, 'Courier New', monospace;"
        " font-size:17px;"
        " font-weight:700;"
        "}"
        "QTextBrowser a {"
        " color:#1A69D8;"
        " text-decoration:underline;"
        " font-weight:900;"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(8);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_subtitleLabel);
    layout->addWidget(m_propertyLabel);
    layout->addWidget(m_statCard, 0);
    layout->addWidget(m_codeLabel);
    layout->addWidget(m_codeView, 1);

    connect(m_codeView, &QTextBrowser::anchorClicked, this, &InfoCodePanel::handleAnchorClicked);
    showEmpty();
}

void InfoCodePanel::showCreature(Creature* creature) {
    if (!creature) {
        showEmpty();
        return;
    }

    m_titleLabel->setText(creature->name());
    m_subtitleLabel->setText(QString("%1  ·  class %2").arg(creature->id(), creature->className()));

    int hp = creature->hp();
    int maxHp = creature->maxHp();
    int percent = maxHp > 0 ? int(100.0 * hp / maxHp) : 0;
    percent = qBound(0, percent, 100);

    QString hpColor = "#6EDB74";
    if (percent <= 30) hpColor = "#FF6B6B";
    else if (percent <= 60) hpColor = "#FFD36E";

    m_hpBar->setValue(percent);
    m_hpBar->setFormat(QString("%1 / %2").arg(hp).arg(maxHp));
    m_hpBar->setStyleSheet(QString(
        "QProgressBar {"
        " background-color:#EEF3FF;"
        " border:2px solid #AFC0F5;"
        " border-radius:9px;"
        " text-align:center;"
        " color:#22315C;"
        " font-weight:900;"
        " font-size:16px;"
        "}"
        "QProgressBar::chunk {"
        " background-color:%1;"
        " border-radius:7px;"
        "}"
    ).arg(hpColor));

    m_atkLabel->setText(QString::number(creature->atk()));
    m_campLabel->setText(creature->isEnemy() ? "Enemy" : "Player");

    m_codeView->setHtml(creature->classCodeHtml());
    m_codeView->verticalScrollBar()->setValue(0);
}

void InfoCodePanel::showEmpty() {
    m_titleLabel->setText("未选中");
    m_subtitleLabel->setText("点击对象查看完整实现");
    m_hpBar->setValue(0);
    m_hpBar->setFormat("- / -");
    m_atkLabel->setText("ATK -");
    m_campLabel->setText("-");
    m_codeView->setHtml("<pre>这里会显示类定义、成员函数签名和关键实现。</pre>");
}

void InfoCodePanel::handleAnchorClicked(const QUrl& url) {
    QString command = url.toString();
    if (command.startsWith("cmd:")) {
        command.remove(0, 4);
    }
    emit commandChosen(QUrl::fromPercentEncoding(command.toUtf8()));
}
