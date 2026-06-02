#pragma once

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QStyleOptionGraphicsItem>
#include <QPointF>

class ActorItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

private:
    QString m_actorId;
    bool m_hovered = false;
    bool m_classHighlighted = false;
    bool m_parameterPickMode = false;

public:
    ActorItem(const QString& actorId, const QPixmap& pixmap);

    QString actorId() const;
    void setClassHighlighted(bool highlighted);
    void setParameterPickMode(bool enabled);

    void playAttackToward(const QPointF& targetPos);
    void playHit();
    void playHeal();

signals:
    void clicked(const QString& actorId);
    void hovered(const QString& actorId);
    void hoverLeft(const QString& actorId);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
};
