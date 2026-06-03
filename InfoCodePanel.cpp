#include "InfoCodePanel.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QScrollBar>
#include <QTextOption>
#include <QVBoxLayout>
#include <QtGlobal>
#include <algorithm>

static QLabel* makeSmallTitle(const QString& text) {
    QLabel* label = new QLabel(text);
    label->setMinimumWidth(46);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label->setStyleSheet(
        "color:#4967C8;"
        "font-weight:900;"
        "font-size:17px;"
        "background:transparent;"
        "border:0;"
        "padding:0;"
    );
    return label;
}

static QString hpBarStyle(const QString& hpColor) {
    return QString(
        "QProgressBar#hpBar {"
        " background-color:#F8FAFF;"
        " border:2px solid #B7C5EA;"
        " border-radius:12px;"
        " text-align:center;"
        " color:#000000;"
        " font-weight:900;"
        " font-size:17px;"
        "}"
        "QProgressBar#hpBar::chunk {"
        " background-color:%1;"
        " border-radius:9px;"
        " margin:3px;"
        "}"
    ).arg(hpColor);
}

static QString classButtonStyle(bool active) {
    if (active) {
        return QString(
            "QPushButton {"
            " background:#E9E6FF;"
            " color:#352A9B;"
            " border:2px solid #6C5BFF;"
            " border-radius:12px;"
            " font-size:15px;"
            " font-weight:900;"
            " padding:6px 10px;"
            "}"
            "QPushButton:hover { background:#F0EEFF; }"
        );
    }
    return QString(
        "QPushButton {"
        " background:#FFFFFF;"
        " color:#22315C;"
        " border:2px solid #AFC0F5;"
        " border-radius:12px;"
        " font-size:15px;"
        " font-weight:900;"
        " padding:6px 10px;"
        "}"
        "QPushButton:hover {"
        " background:#F6F8FF;"
        " border:2px solid #7E8DFF;"
        "}"
    );
}

InfoCodePanel::InfoCodePanel(QWidget* parent)
    : QWidget(parent) {
    setObjectName("infoCodePanel");

    m_titleLabel = new QLabel("未选中");
    m_subtitleLabel = new QLabel("点击对象查看实现");
    m_introLabel = new QLabel("介绍：-");
    m_propertyLabel = new QLabel("状态");
    m_codeLabel = new QLabel("源码");
    m_classTitleLabel = new QLabel("查看类代码");
    m_classCard = new QFrame;
    m_statCard = new QFrame;
    m_hpBar = new QProgressBar;
    m_atkLabel = new QLabel("ATK -");
    m_campLabel = new QLabel("-");
    m_codeView = new QTextBrowser;

    QFrame* headerCard = new QFrame;
    headerCard->setObjectName("infoHeaderCard");
    headerCard->setFixedHeight(112);

    m_classCard->setObjectName("infoClassCard");
    m_statCard->setObjectName("infoStatCard");
    m_statCard->setFixedHeight(122);

    m_hpBar->setObjectName("hpBar");
    m_codeView->setObjectName("codeView");

    setStyleSheet(
        "#infoCodePanel {"
        " background:qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F4F7FF, stop:1 #E8EFFF);"
        " border:2px solid rgba(174, 193, 241, 190);"
        " border-radius:18px;"
        "}"
        "QFrame#infoHeaderCard, QFrame#infoStatCard, QFrame#infoClassCard {"
        " background:qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F7F9FF);"
        " border:2px solid #AFC0F5;"
        " border-radius:16px;"
        "}"
        "QLabel {"
        " background:transparent;"
        " border:0;"
        "}"
        "QTextBrowser#codeView {"
        " background:#FFFFFF;"
        " color:#22315C;"
        " border:2px solid #AFC0F5;"
        " border-radius:16px;"
        " padding:12px;"
        " selection-background-color:#5E72FF;"
        " font-family:Consolas, 'Courier New', monospace;"
        " font-size:17px;"
        " font-weight:700;"
        "}"
        "QTextBrowser#codeView a {"
        " color:#1A69D8;"
        " text-decoration:underline;"
        " font-weight:900;"
        "}"
        "QScrollBar:vertical {"
        " background:#EEF3FF;"
        " width:10px;"
        " border:0;"
        " margin:2px 0 2px 0;"
        " border-radius:5px;"
        "}"
        "QScrollBar::handle:vertical {"
        " background:#AFC0F5;"
        " min-height:26px;"
        " border-radius:5px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        " background:#8FA6EA;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        " height:0;"
        " background:transparent;"
        " border:0;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        " background:transparent;"
        "}"
    );

    m_classTitleLabel->setStyleSheet(
        "color:#3656B2;"
        "font-size:17px;"
        "font-weight:900;"
        "padding:0;"
        "background:transparent;"
        "border:0;"
    );
    m_titleLabel->setStyleSheet(
        "color:#17234D;"
        "font-size:24px;"
        "font-weight:900;"
        "padding:0;"
        "background:transparent;"
        "border:0;"
    );
    m_subtitleLabel->setStyleSheet(
        "color:#52679A;"
        "font-size:17px;"
        "font-weight:800;"
        "padding:0;"
        "background:transparent;"
        "border:0;"
    );
    m_introLabel->setWordWrap(true);
    m_introLabel->setStyleSheet(
        "color:#415681;"
        "font-size:15px;"
        "font-weight:700;"
        "padding:0;"
        "background:transparent;"
        "border:0;"
    );
    m_propertyLabel->setStyleSheet(
        "color:#3656B2;"
        "font-size:18px;"
        "font-weight:900;"
        "padding:0 0 0 2px;"
        "background:transparent;"
        "border:0;"
    );
    m_codeLabel->setStyleSheet(
        "color:#7B4C00;"
        "font-size:18px;"
        "font-weight:900;"
        "padding:0 0 0 2px;"
        "background:transparent;"
        "border:0;"
    );

    m_hpBar->setRange(0, 100);
    m_hpBar->setFixedHeight(34);
    m_hpBar->setTextVisible(true);
    m_hpBar->setFormat("- / -");
    m_hpBar->setStyleSheet(hpBarStyle("#CBD5EA"));

    m_atkLabel->setStyleSheet("color:#17234D; font-size:20px; font-weight:900; background:transparent; border:0; padding:0;");
    m_campLabel->setStyleSheet("color:#7B4C00; font-size:20px; font-weight:900; background:transparent; border:0; padding:0;");

    m_classButtonLayout = new QGridLayout;
    m_classButtonLayout->setContentsMargins(0, 0, 0, 0);
    m_classButtonLayout->setHorizontalSpacing(6);
    m_classButtonLayout->setVerticalSpacing(6);

    QVBoxLayout* classLayout = new QVBoxLayout(m_classCard);
    classLayout->setContentsMargins(14, 10, 14, 10);
    classLayout->setSpacing(7);
    classLayout->addWidget(m_classTitleLabel);
    classLayout->addLayout(m_classButtonLayout);

    QVBoxLayout* headerLayout = new QVBoxLayout(headerCard);
    headerLayout->setContentsMargins(16, 10, 16, 10);
    headerLayout->setSpacing(3);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_subtitleLabel);
    headerLayout->addWidget(m_introLabel);

    QGridLayout* statGrid = new QGridLayout;
    statGrid->setContentsMargins(0, 0, 0, 0);
    statGrid->setHorizontalSpacing(10);
    statGrid->setVerticalSpacing(6);
    statGrid->addWidget(makeSmallTitle("HP"), 0, 0);
    statGrid->addWidget(m_hpBar, 0, 1);
    statGrid->addWidget(makeSmallTitle("ATK"), 1, 0);
    statGrid->addWidget(m_atkLabel, 1, 1);
    statGrid->addWidget(makeSmallTitle("阵营"), 2, 0);
    statGrid->addWidget(m_campLabel, 2, 1);

    QVBoxLayout* cardLayout = new QVBoxLayout(m_statCard);
    cardLayout->setContentsMargins(14, 10, 14, 10);
    cardLayout->setSpacing(0);
    cardLayout->addLayout(statGrid);

    m_codeView->setOpenLinks(false);
    m_codeView->setOpenExternalLinks(false);
    m_codeView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_codeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QTextOption option = m_codeView->document()->defaultTextOption();
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_codeView->document()->setDefaultTextOption(option);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(6);
    layout->addWidget(m_classCard);
    layout->addWidget(headerCard);
    layout->addWidget(m_propertyLabel);
    layout->addWidget(m_statCard, 0);
    layout->addWidget(m_codeLabel);
    layout->addWidget(m_codeView, 1);

    connect(m_codeView, &QTextBrowser::anchorClicked, this, &InfoCodePanel::handleAnchorClicked);
    showEmpty();
}

void InfoCodePanel::setClassChoices(const QList<Creature*>& creatures, int level) {
    m_classCodeByName.clear();
    m_activeClassName.clear();

    QLayoutItem* child = nullptr;
    while ((child = m_classButtonLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }
    m_classButtons.clear();

    QList<QString> classNames;
    m_classCodeByName.insert("Creature", Creature::baseClassCodeHtml(level));
    classNames << "Creature";

    for (Creature* creature : creatures) {
        if (!creature) continue;
        const QString className = creature->className();
        if (className.isEmpty() || m_classCodeByName.contains(className)) continue;
        m_classCodeByName.insert(className, creature->classCodeHtml());
        classNames << className;
    }

    std::sort(classNames.begin(), classNames.end());
    const int columns = 3;
    for (int i = 0; i < classNames.size(); ++i) {
        const QString className = classNames[i];
        QPushButton* button = new QPushButton(className);
        button->setMinimumHeight(34);
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(classButtonStyle(false));
        connect(button, &QPushButton::clicked, this, [this, className]() {
            showClassCode(className);
            emit classSelected(className);
        });
        m_classButtons.push_back(button);
        m_classButtonLayout->addWidget(button, i / columns, i % columns);
    }

    m_classCard->setVisible(!classNames.isEmpty());
    updateClassButtons();
}

void InfoCodePanel::updateClassButtons() {
    for (QPushButton* button : m_classButtons) {
        if (!button) continue;
        button->setStyleSheet(classButtonStyle(button->text() == m_activeClassName));
    }
}

void InfoCodePanel::showCreature(Creature* creature) {
    m_activeClassName.clear();
    updateClassButtons();
    m_introLabel->setVisible(true);

    if (!creature) {
        showEmpty();
        return;
    }

    m_propertyLabel->setVisible(true);
    m_statCard->setVisible(true);
    m_codeLabel->setText("源码");

    m_titleLabel->setText(QString("名称：%1").arg(creature->id()));
    m_subtitleLabel->setText(QString("类名：%1").arg(creature->className()));
    m_introLabel->setText(QString("介绍：%1").arg(creature->description()));

    int hp = creature->hp();
    int maxHp = creature->maxHp();
    int percent = maxHp > 0 ? int(100.0 * hp / maxHp) : 0;
    percent = qBound(0, percent, 100);

    QString hpColor = "#6EDB74";
    if (percent <= 30) hpColor = "#FF6B6B";
    else if (percent <= 60) hpColor = "#FFD36E";

    m_hpBar->setValue(percent);
    m_hpBar->setFormat(QString("%1 / %2").arg(hp).arg(maxHp));
    m_hpBar->setStyleSheet(hpBarStyle(hpColor));

    m_atkLabel->setText(QString::number(creature->atk()));
    m_campLabel->setText(creature->isEnemy() ? "敌对" : "友好");

    m_codeView->setHtml(creature->classCodeHtml());
    m_codeView->verticalScrollBar()->setValue(0);
}

void InfoCodePanel::showClassCode(const QString& className) {
    m_activeClassName = className;
    updateClassButtons();

    m_titleLabel->setText("类代码");
    m_subtitleLabel->setText(QString("类名：%1").arg(className));
    m_introLabel->clear();
    m_introLabel->setVisible(false);

    m_propertyLabel->setVisible(false);
    m_statCard->setVisible(false);
    m_codeLabel->setText("源码");

    const QString html = m_classCodeByName.value(className, QString("<pre>没有找到这个类的代码。</pre>"));
    m_codeView->setHtml(html);
    m_codeView->verticalScrollBar()->setValue(0);
}

void InfoCodePanel::showEmpty() {
    m_activeClassName.clear();
    updateClassButtons();
    m_introLabel->setVisible(true);

    m_propertyLabel->setVisible(true);
    m_statCard->setVisible(true);
    m_codeLabel->setText("源码");

    m_titleLabel->setText("未选中");
    m_subtitleLabel->setText("点击对象查看完整实现");
    m_introLabel->setText("介绍：-");
    m_hpBar->setValue(0);
    m_hpBar->setFormat("- / -");
    m_hpBar->setStyleSheet(hpBarStyle("#CBD5EA"));
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
