#pragma once

#include <QPlainTextEdit>
#include <QPoint>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <QString>

class CodeInputEdit : public QPlainTextEdit {
    Q_OBJECT
private:
    int m_hoverQuestionPos = -1;
    int m_activeQuestionPos = -1;
    bool m_previewActive = false;
    int m_previewStart = -1;
    int m_previewLength = 0;
    bool m_internalChange = false;

    int questionPosAt(const QPoint& pos) const;
    QPoint questionGlobalCenter(int charPos) const;
    void updateQuestionHighlights();
    void clearPreviewInternal();

public:
    explicit CodeInputEdit(QWidget* parent = nullptr);

    QString commandText() const;
    void setCommandText(const QString& text);
    bool hasActiveQuestion() const;
    QPoint activeQuestionGlobalCenter() const;

public slots:
    void previewParameter(const QString& objectName);
    void clearParameterPreview();
    void commitParameter(const QString& objectName);
    void cancelParameterSelection();

signals:
    void returnPressed();
    void parameterSelectionChanged(bool active, const QPoint& globalStart);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
};

class CommandPanel : public QWidget {
    Q_OBJECT
private:
    CodeInputEdit* m_input = nullptr;
    QPushButton* m_runButton = nullptr;
    QTextEdit* m_logView = nullptr;

public:
    explicit CommandPanel(QWidget* parent = nullptr);

    void setCommand(const QString& command);
    void appendLog(const QString& text);
    void clearLog();
    bool isParameterSelectionActive() const;

public slots:
    void previewParameter(const QString& objectName);
    void clearParameterPreview();
    void commitParameter(const QString& objectName);
    void cancelParameterSelection();

signals:
    void commandSubmitted(const QString& command);
    void parameterSelectionChanged(bool active, const QPoint& globalStart);

private slots:
    void submit();
};
