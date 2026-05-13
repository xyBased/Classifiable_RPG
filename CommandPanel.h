#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>

class CommandPanel : public QWidget {
    Q_OBJECT

private:
    QLineEdit* m_input = nullptr;
    QPushButton* m_runButton = nullptr;
    QTextEdit* m_logView = nullptr;

public:
    explicit CommandPanel(QWidget* parent = nullptr);

    void setCommand(const QString& command);
    void appendLog(const QString& text);

signals:
    void commandSubmitted(const QString& command);

private slots:
    void submit();
};
