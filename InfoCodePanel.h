#pragma once

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QProgressBar>
#include <QPushButton>
#include <QTextBrowser>
#include <QUrl>
#include <QVector>
#include <QWidget>

#include "Creature.h"

class InfoCodePanel : public QWidget {
    Q_OBJECT
private:
    QLabel* m_titleLabel = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    QLabel* m_introLabel = nullptr;
    QLabel* m_propertyLabel = nullptr;
    QLabel* m_codeLabel = nullptr;
    QLabel* m_classTitleLabel = nullptr;
    QFrame* m_classCard = nullptr;
    QFrame* m_statCard = nullptr;
    QProgressBar* m_hpBar = nullptr;
    QLabel* m_atkLabel = nullptr;
    QLabel* m_campLabel = nullptr;
    QTextBrowser* m_codeView = nullptr;
    QGridLayout* m_classButtonLayout = nullptr;
    QVector<QPushButton*> m_classButtons;
    QMap<QString, QString> m_classCodeByName;
    QString m_activeClassName;

    void updateClassButtons();

public:
    explicit InfoCodePanel(QWidget* parent = nullptr);

    void setClassChoices(const QList<Creature*>& creatures, int level = 0);
    void showCreature(Creature* creature);
    void showClassCode(const QString& className);
    void showEmpty();

signals:
    void commandChosen(const QString& command);
    void classSelected(const QString& className);

private slots:
    void handleAnchorClicked(const QUrl& url);
};
