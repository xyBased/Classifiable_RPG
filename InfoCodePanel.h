#pragma once

#include <QWidget>
#include <QLabel>
#include <QTextBrowser>
#include <QProgressBar>
#include <QFrame>

#include "Creature.h"

class InfoCodePanel : public QWidget {
    Q_OBJECT

private:
    QLabel* m_titleLabel = nullptr;
    QLabel* m_propertyLabel = nullptr;
    QLabel* m_codeLabel = nullptr;

    QFrame* m_propertyCard = nullptr;

    QProgressBar* m_hpBar = nullptr;

    QLabel* m_idValueLabel = nullptr;
    QLabel* m_classValueLabel = nullptr;
    QLabel* m_atkValueLabel = nullptr;
    QLabel* m_campValueLabel = nullptr;

    QTextBrowser* m_codeView = nullptr;

public:
    explicit InfoCodePanel(QWidget* parent = nullptr);

    void showCreature(Creature* creature);
    void showEmpty();

signals:
    void commandChosen(const QString& command);

private slots:
    void handleAnchorClicked(const QUrl& url);
};
