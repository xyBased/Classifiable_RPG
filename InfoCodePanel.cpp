#include "InfoCodePanel.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QTextBrowser>
#include <QUrl>
#include <QVBoxLayout>

namespace {

QLabel* makeNameLabel(const QString& text) {
    QLabel* label = new QLabel(text);
    label->setStyleSheet("color:#A9B7D0; font-weight:600;");
    return label;
}

QLabel* makeValueLabel() {
    QLabel* label = new QLabel("-");
    label->setStyleSheet("color:#F4F7FF; font-weight:600;");
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

} // namespace

InfoCodePanel::InfoCodePanel(QWidget* parent)
    : QWidget(parent) {
    setMinimumWidth(360);
    setStyleSheet(
        "InfoCodePanel { background:#202436; }"
        "QLabel { font-family:'Microsoft YaHei UI'; }"
        "QTextBrowser { background:#141824; color:#EAF1FF; border:1px solid #3D4668; border-radius:8px; padding:8px; }"
        "QProgressBar { background:#121622; border:1px solid #3D4668; border-radius:7px; color:white; text-align:center; height:16px; }"
        "QProgressBar::chunk { background:#69DB7C; border-radius:6px; }"
        );

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    m_titleLabel = new QLabel("未选择对象");
    m_titleLabel->setStyleSheet("color:#FFFFFF; font-size:18px; font-weight:800;");
    root->addWidget(m_titleLabel);

    m_propertyCard = new QFrame;
    m_propertyCard->setFrameShape(QFrame::StyledPanel);
    m_propertyCard->setStyleSheet("QFrame { background:#272D44; border:1px solid #3D4668; border-radius:10px; }");
    QVBoxLayout* cardLayout = new QVBoxLayout(m_propertyCard);
    cardLayout->setContentsMargins(12, 10, 12, 10);
    cardLayout->setSpacing(8);

    m_hpBar = new QProgressBar;
    m_hpBar->setRange(0, 1);
    m_hpBar->setValue(0);
    cardLayout->addWidget(m_hpBar);

    QGridLayout* grid = new QGridLayout;
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(6);

    m_idValueLabel = makeValueLabel();
    m_classValueLabel = makeValueLabel();
    m_atkValueLabel = makeValueLabel();
    m_campValueLabel = makeValueLabel();

    grid->addWidget(makeNameLabel("id"), 0, 0);
    grid->addWidget(m_idValueLabel, 0, 1);
    grid->addWidget(makeNameLabel("class"), 1, 0);
    grid->addWidget(m_classValueLabel, 1, 1);
    grid->addWidget(makeNameLabel("atk"), 2, 0);
    grid->addWidget(m_atkValueLabel, 2, 1);
    grid->addWidget(makeNameLabel("camp"), 3, 0);
    grid->addWidget(m_campValueLabel, 3, 1);
    cardLayout->addLayout(grid);

    m_propertyLabel = new QLabel;
    m_propertyLabel->setStyleSheet("color:#D9E2F2; line-height:1.3;");
    m_propertyLabel->setWordWrap(true);
    m_propertyLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    cardLayout->addWidget(m_propertyLabel);

    root->addWidget(m_propertyCard);

    m_codeLabel = new QLabel("类代码 / 可点击命令");
    m_codeLabel->setStyleSheet("color:#FFFFFF; font-size:15px; font-weight:700;");
    root->addWidget(m_codeLabel);

    m_codeView = new QTextBrowser;
    m_codeView->setOpenLinks(false);
    m_codeView->setOpenExternalLinks(false);
    m_codeView->setMinimumHeight(260);
    m_codeView->setTextInteractionFlags(Qt::TextBrowserInteraction | Qt::TextSelectableByMouse);
    root->addWidget(m_codeView, 1);

    connect(m_codeView, &QTextBrowser::anchorClicked, this, &InfoCodePanel::handleAnchorClicked);

    showEmpty();
}

void InfoCodePanel::showCreature(Creature* creature) {
    if (!creature) {
        showEmpty();
        return;
    }

    m_titleLabel->setText(creature->name());
    m_idValueLabel->setText(creature->id());
    m_classValueLabel->setText(creature->className());
    m_atkValueLabel->setText(QString::number(creature->atk()));
    m_campValueLabel->setText(creature->campText());

    m_hpBar->setRange(0, creature->maxHp());
    m_hpBar->setValue(creature->hp());
    m_hpBar->setFormat(QString("HP %1 / %2").arg(creature->hp()).arg(creature->maxHp()));

    m_propertyLabel->setText(creature->propertyText());
    m_codeView->setHtml(creature->classCodeHtml());
}

void InfoCodePanel::showEmpty() {
    m_titleLabel->setText("未选择对象");
    m_idValueLabel->setText("-");
    m_classValueLabel->setText("-");
    m_atkValueLabel->setText("-");
    m_campValueLabel->setText("-");
    m_hpBar->setRange(0, 1);
    m_hpBar->setValue(0);
    m_hpBar->setFormat("HP - / -");
    m_propertyLabel->setText("点击左侧对象查看属性与类代码。");
    m_codeView->setHtml(
        "<div style='color:#A9B7D0; font-family:Microsoft YaHei UI;'>"
        "点击场景中的对象后，这里会显示它的类定义。"
        "</div>"
        );
}

void InfoCodePanel::handleAnchorClicked(const QUrl& url) {
    QString s = url.toString();
    if (!s.startsWith("cmd:")) return;

    QString command = s.mid(4);
    command = QUrl::fromPercentEncoding(command.toUtf8());
    emit commandChosen(command);
}
