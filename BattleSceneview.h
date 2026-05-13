#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMap>

#include "GameLevel.h"
#include "ActorItem.h"

class BattleSceneView : public QGraphicsView {
    Q_OBJECT

private:
    QGraphicsScene* m_scene = nullptr;
    GameLevel* m_level = nullptr;
    QMap<QString, ActorItem*> m_actorItems;
    QString m_selectedActorId;

    void updateSelection();
    void addHpBar(Creature* creature, const QPointF& footPos);

public:
    explicit BattleSceneView(QWidget* parent = nullptr);

    void setLevel(GameLevel* level);
    void refresh();

    void setSelectedActor(const QString& actorId);

signals:
    void actorSelected(const QString& actorId);
};
