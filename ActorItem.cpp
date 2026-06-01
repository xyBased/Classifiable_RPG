#include "ActorItem.h"

#include <QCursor>
#include <QPainter>
#include <QPen>

ActorItem::ActorItem(const QString& actorId, const QPixmap& pixmap)
    : QObject(), QGraphicsPixmapItem(pixmap), m_actorId(actorId) {
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setCursor(Qt::PointingHandCursor);
    setShapeMode(QGraphicsPixmapItem::MaskShape);
    setTransformationMode(Qt::SmoothTransformation);
    setOffset(-pixmap.width() / 2.0, -pixmap.height());
}

QString ActorItem::actorId() const { return m_actorId; }

void ActorItem::playAttackToward(const QPointF&) {}
void ActorItem::playHit() {}
void ActorItem::playHeal() {}

void ActorItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    QGraphicsPixmapItem::paint(painter, option, widget);

    QPainterPath spritePath = QGraphicsPixmapItem::shape();

    if (m_hovered && !isSelected()) {
        painter->setPen(QPen(QColor(120, 190, 255, 220), 4));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(spritePath);
    }

    if (isSelected()) {
        painter->setPen(QPen(QColor(255, 214, 92, 235), 5));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(spritePath);
    }
}

void ActorItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    setSelected(true);
    emit clicked(m_actorId);
    QGraphicsPixmapItem::mousePressEvent(event);
}

void ActorItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    m_hovered = true;
    update();
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

void ActorItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    m_hovered = false;
    update();
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}
