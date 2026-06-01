#pragma once

#include <QFrame>
#include <QLabel>
#include <QProgressBar>
#include <QTextBrowser>
#include <QWidget>
#include <QUrl>

#include "Creature.h"

class InfoCodePanel : public QWidget {
    Q_OBJECT
private:
    QLabel* m_titleLabel = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    QLabel* m_propertyLabel = nullptr;
    QLabel* m_codeLabel = nullptr;
    QFrame* m_statCard = nullptr;
    QProgressBar* m_hpBar = nullptr;
    QLabel* m_atkLabel = nullptr;
    QLabel* m_campLabel = nullptr;
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
