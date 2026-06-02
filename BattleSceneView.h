#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMap>
#include <QPointF>
#include <QMouseEvent>

#include "GameLevel.h"
#include "ActorItem.h"

class BattleSceneView : public QGraphicsView {
    Q_OBJECT
private:
    QGraphicsScene* m_scene = nullptr;
    GameLevel* m_level = nullptr;
    QMap<QString, ActorItem*> m_actorItems;
    QMap<QString, QPointF> m_actorPositions;
    QString m_selectedActorId = "player";
    QString m_classHighlightName;
    bool m_parameterPickMode = false;

public:
    explicit BattleSceneView(QWidget* parent = nullptr);

    void setLevel(GameLevel* level);
    void setSelectedActor(const QString& actorId);
    void setClassHighlight(const QString& className);
    void clearClassHighlight();
    void setParameterPickMode(bool enabled);
    bool parameterPickMode() const;

    void playAttack(const QString& sourceId, const QString& targetId);
    void playHit(const QString& actorId);
    void playHeal(const QString& actorId);

signals:
    void actorSelected(const QString& actorId);
    void actorHovered(const QString& actorId);
    void actorHoverLeft(const QString& actorId);
    void actorParameterPicked(const QString& actorId);
    void actorParameterPickCanceled();

public slots:
    void refresh();

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    void addBackground();
    void addHud();
    void addHpBar(Creature* creature, const QPointF& footPos, int width);
    void addIntent(Creature* creature, const QPointF& footPos, int spriteHeight);
    QPixmap spriteFor(Creature* creature, int& targetHeight) const;
    void updateSelection();
};
