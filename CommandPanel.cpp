#include "CommandPanel.h"

CommandPanel::CommandPanel(QWidget* parent)
    : QWidget(parent) {

    m_input = new QLineEdit;
    m_runButton = new QPushButton("执行");
    m_logView = new QTextEdit;

    m_logView->setReadOnly(true);

    QHBoxLayout* inputLayout = new QHBoxLayout;
    inputLayout->addWidget(m_input);
    inputLayout->addWidget(m_runButton);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_logView);

    connect(m_runButton, &QPushButton::clicked,
            this, &CommandPanel::submit);

    connect(m_input, &QLineEdit::returnPressed,
            this, &CommandPanel::submit);
}

void CommandPanel::setCommand(const QString& command) {
    m_input->setText(command);
    m_input->setFocus();
}

void CommandPanel::appendLog(const QString& text) {
    m_logView->append(text);
}

void CommandPanel::submit() {
    QString command = m_input->text().trimmed();

    if (command.isEmpty()) {
        return;
    }

    emit commandSubmitted(command);
}
