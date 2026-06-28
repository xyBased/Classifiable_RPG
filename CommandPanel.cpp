#include "CommandPanel.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QtGlobal>

CodeInputEdit::CodeInputEdit(QWidget* parent)
    : QPlainTextEdit(parent) {
    setObjectName("codeInputEdit");
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabChangesFocus(true);
    setFixedHeight(58);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    setPlaceholderText("输入代码指令");

    connect(this, &QPlainTextEdit::textChanged, this, [this]() {
        if (!m_internalChange) {
            m_previewActive = false;
            m_previewStart = -1;
            m_previewLength = 0;

            const QString text = toPlainText();
            if (m_activeQuestionPos < 0 || m_activeQuestionPos >= text.size() || text[m_activeQuestionPos] != '?') {
                if (m_activeQuestionPos >= 0) {
                    m_activeQuestionPos = -1;
                    emit parameterSelectionChanged(false, QPoint());
                }
            }
        }
        updateQuestionHighlights();
    });

    updateQuestionHighlights();
}

QString CodeInputEdit::commandText() const {
    QString text = toPlainText();
    text.replace('\n', ' ');
    return text.trimmed();
}

void CodeInputEdit::setCommandText(const QString& text) {
    m_internalChange = true;
    setPlainText(text);
    m_internalChange = false;

    m_hoverQuestionPos = -1;
    m_activeQuestionPos = -1;
    m_previewActive = false;
    m_previewStart = -1;
    m_previewLength = 0;
    moveCursor(QTextCursor::End);
    updateQuestionHighlights();
    emit parameterSelectionChanged(false, QPoint());
    setFocus();
}

bool CodeInputEdit::hasActiveQuestion() const {
    return m_activeQuestionPos >= 0;
}

QPoint CodeInputEdit::activeQuestionGlobalCenter() const {
    if (m_activeQuestionPos < 0) return QPoint();
    return questionGlobalCenter(m_activeQuestionPos);
}

int CodeInputEdit::questionPosAt(const QPoint& pos) const {
    const QString text = toPlainText();
    if (text.isEmpty()) return -1;

    QTextCursor c = cursorForPosition(pos);
    int p = c.position();
    if (p >= 0 && p < text.size() && text[p] == '?') return p;
    if (p > 0 && p - 1 < text.size() && text[p - 1] == '?') {
        return p - 1;
    }
    return -1;
}

QPoint CodeInputEdit::questionGlobalCenter(int charPos) const {
    if (charPos < 0) return QPoint();

    QTextCursor c(document());
    c.setPosition(qMin<int>(charPos, toPlainText().size()));
    QRect r = cursorRect(c);
    const int charWidth = qMax(10, fontMetrics().horizontalAdvance(QStringLiteral("?")));
    QPoint local(r.left() + charWidth / 2, r.center().y());
    return viewport()->mapToGlobal(local);
}

void CodeInputEdit::updateQuestionHighlights() {
    QList<QTextEdit::ExtraSelection> selections;
    const QString text = toPlainText();

    for (int i = 0; i < text.size(); ++i) {
        if (text[i] != '?') continue;
        QTextEdit::ExtraSelection s;
        s.cursor = QTextCursor(document());
        s.cursor.setPosition(i);
        s.cursor.setPosition(i + 1, QTextCursor::KeepAnchor);
        s.format.setForeground(QColor(255, 244, 180));
        s.format.setBackground(QColor(78, 108, 255, 210));
        s.format.setFontWeight(QFont::Black);
        if (i == m_hoverQuestionPos || i == m_activeQuestionPos) {
            s.format.setForeground(QColor(40, 36, 95));
            s.format.setBackground(QColor(255, 218, 92, 235));
        }
        selections << s;
    }

    if (m_previewActive && m_previewStart >= 0 && m_previewLength > 0) {
        QTextEdit::ExtraSelection p;
        p.cursor = QTextCursor(document());
        p.cursor.setPosition(m_previewStart);
        p.cursor.setPosition(qMin<int>(m_previewStart + m_previewLength, text.size()), QTextCursor::KeepAnchor);
        p.format.setForeground(QColor(70, 90, 140, 135));
        p.format.setBackground(QColor(194, 216, 255, 95));
        p.format.setFontItalic(true);
        selections << p;
    }

    setExtraSelections(selections);
}

void CodeInputEdit::clearPreviewInternal() {
    if (!m_previewActive) return;

    m_internalChange = true;
    QTextCursor c(document());
    c.setPosition(qBound<int>(0, m_previewStart, toPlainText().size()));
    c.setPosition(qBound<int>(0, m_previewStart + m_previewLength, toPlainText().size()), QTextCursor::KeepAnchor);
    c.insertText(QStringLiteral("?"));
    m_internalChange = false;

    m_activeQuestionPos = qBound<int>(0, m_previewStart, toPlainText().size() - 1);
    m_previewActive = false;
    m_previewStart = -1;
    m_previewLength = 0;
}

void CodeInputEdit::previewParameter(const QString& objectName) {
    if (m_activeQuestionPos < 0 || objectName.isEmpty()) return;

    clearPreviewInternal();

    const QString text = toPlainText();
    if (m_activeQuestionPos < 0 || m_activeQuestionPos >= text.size() || text[m_activeQuestionPos] != '?') return;

    m_internalChange = true;
    QTextCursor c(document());
    c.setPosition(m_activeQuestionPos);
    c.setPosition(m_activeQuestionPos + 1, QTextCursor::KeepAnchor);
    c.insertText(objectName);
    m_internalChange = false;

    m_previewActive = true;
    m_previewStart = m_activeQuestionPos;
    m_previewLength = objectName.size();
    updateQuestionHighlights();
    emit parameterSelectionChanged(true, activeQuestionGlobalCenter());
}

void CodeInputEdit::clearParameterPreview() {
    if (!m_previewActive) return;
    clearPreviewInternal();
    updateQuestionHighlights();
    if (m_activeQuestionPos >= 0) emit parameterSelectionChanged(true, activeQuestionGlobalCenter());
}

void CodeInputEdit::commitParameter(const QString& objectName) {
    if (m_activeQuestionPos < 0 || objectName.isEmpty()) return;

    m_internalChange = true;
    QTextCursor c(document());
    if (m_previewActive) {
        c.setPosition(qBound<int>(0, m_previewStart, toPlainText().size()));
        c.setPosition(qBound<int>(0, m_previewStart + m_previewLength, toPlainText().size()), QTextCursor::KeepAnchor);
    } else {
        const QString text = toPlainText();
        if (m_activeQuestionPos < 0 || m_activeQuestionPos >= text.size() || text[m_activeQuestionPos] != '?') {
            m_internalChange = false;
            cancelParameterSelection();
            return;
        }
        c.setPosition(m_activeQuestionPos);
        c.setPosition(m_activeQuestionPos + 1, QTextCursor::KeepAnchor);
    }
    c.insertText(objectName);
    setTextCursor(c);
    m_internalChange = false;

    m_hoverQuestionPos = -1;
    m_activeQuestionPos = -1;
    m_previewActive = false;
    m_previewStart = -1;
    m_previewLength = 0;
    updateQuestionHighlights();
    emit parameterSelectionChanged(false, QPoint());
    setFocus();
}

void CodeInputEdit::cancelParameterSelection() {
    clearPreviewInternal();
    m_hoverQuestionPos = -1;
    m_activeQuestionPos = -1;
    updateQuestionHighlights();
    emit parameterSelectionChanged(false, QPoint());
}

void CodeInputEdit::keyPressEvent(QKeyEvent* event) {
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && !(event->modifiers() & Qt::ShiftModifier)) {
        event->accept();
        emit returnPressed();
        return;
    }
    QPlainTextEdit::keyPressEvent(event);
}

void CodeInputEdit::mouseMoveEvent(QMouseEvent* event) {
    const int pos = questionPosAt(event->pos());
    if (pos != m_hoverQuestionPos) {
        m_hoverQuestionPos = pos;
        viewport()->setCursor(pos >= 0 ? Qt::PointingHandCursor : Qt::IBeamCursor);
        updateQuestionHighlights();
    }
    QPlainTextEdit::mouseMoveEvent(event);
}

void CodeInputEdit::mousePressEvent(QMouseEvent* event) {
    const int pos = questionPosAt(event->pos());
    if (pos >= 0) {
        clearPreviewInternal();
        m_activeQuestionPos = pos;
        m_hoverQuestionPos = pos;
        QTextCursor c(document());
        c.setPosition(pos + 1);
        setTextCursor(c);
        updateQuestionHighlights();
        emit parameterSelectionChanged(true, activeQuestionGlobalCenter());
        event->accept();
        return;
    }

    cancelParameterSelection();
    QPlainTextEdit::mousePressEvent(event);
}

void CodeInputEdit::leaveEvent(QEvent* event) {
    if (m_hoverQuestionPos >= 0) {
        m_hoverQuestionPos = -1;
        updateQuestionHighlights();
    }
    viewport()->unsetCursor();
    QPlainTextEdit::leaveEvent(event);
}

CommandPanel::CommandPanel(QWidget* parent)
    : QWidget(parent) {
    m_input = new CodeInputEdit;
    m_runButton = new QPushButton("执行");
    m_logView = new QTextEdit;
    m_logView->setReadOnly(true);

    setStyleSheet(
        "QWidget { background:#EDF2FF; }"
        "QPlainTextEdit#codeInputEdit {"
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
    connect(m_input, &CodeInputEdit::returnPressed, this, &CommandPanel::submit);
    connect(m_input, &CodeInputEdit::parameterSelectionChanged, this, &CommandPanel::parameterSelectionChanged);
}

void CommandPanel::setCommand(const QString& command) {
    m_input->setCommandText(command);
}

void CommandPanel::appendLog(const QString& text) {
    m_logView->append(text);
}

void CommandPanel::clearLog() {
    m_logView->clear();
}

bool CommandPanel::isParameterSelectionActive() const {
    return m_input && m_input->hasActiveQuestion();
}

void CommandPanel::previewParameter(const QString& objectName) {
    if (m_input) m_input->previewParameter(objectName);
}

void CommandPanel::clearParameterPreview() {
    if (m_input) m_input->clearParameterPreview();
}

void CommandPanel::commitParameter(const QString& objectName) {
    if (m_input) m_input->commitParameter(objectName);
}

void CommandPanel::cancelParameterSelection() {
    if (m_input) m_input->cancelParameterSelection();
}

void CommandPanel::submit() {
    if (m_input) m_input->clearParameterPreview();
    QString command = m_input->commandText();
    if (command.isEmpty()) return;
    emit commandSubmitted(command);
}
