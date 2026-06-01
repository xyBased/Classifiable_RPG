#include "CommandPanel.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

CommandPanel::CommandPanel(QWidget* parent)
    : QWidget(parent) {
    m_input = new QLineEdit;
    m_runButton = new QPushButton("执行");
    m_logView = new QTextEdit;
    m_logView->setReadOnly(true);

    setStyleSheet(
        "QWidget { background:#EDF2FF; }"
        "QLineEdit {"
        " background:#FFFFFF;"
        " color:#22315C;"
        " border:2px solid #AFC0F5;"
        " border-radius:12px;"
        " padding:12px;"
        " font-family:Consolas, 'Courier New', monospace;"
        " font-size:20px;"
        " font-weight:700;"
        "}"
        "QPushButton {"
        " background:#526CFF;"
        " color:white;"
        " border:0;"
        " border-radius:12px;"
        " padding:12px 22px;"
        " font-size:18px;"
        " font-weight:900;"
        "}"
        "QPushButton:hover { background:#6880FF; }"
        "QTextEdit {"
        " background:#FFFFFF;"
        " color:#22315C;"
        " border:2px solid #AFC0F5;"
        " border-radius:14px;"
        " padding:12px;"
        " font-family:'Microsoft YaHei UI', Consolas, 'Courier New', monospace;"
        " font-size:18px;"
        " font-weight:700;"
        "}"
    );

    QHBoxLayout* inputLayout = new QHBoxLayout;
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(8);
    inputLayout->addWidget(m_input, 1);
    inputLayout->addWidget(m_runButton, 0);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_logView, 1);

    connect(m_runButton, &QPushButton::clicked, this, &CommandPanel::submit);
    connect(m_input, &QLineEdit::returnPressed, this, &CommandPanel::submit);
}

void CommandPanel::setCommand(const QString& command) {
    m_input->setText(command);
    m_input->setFocus();
}

void CommandPanel::appendLog(const QString& text) {
    m_logView->append(text);
}

void CommandPanel::clearLog() {
    m_logView->clear();
}

void CommandPanel::submit() {
    QString command = m_input->text().trimmed();
    if (command.isEmpty()) return;
    emit commandSubmitted(command);
}
