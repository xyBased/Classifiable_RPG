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
    // 使用透明遮罩做命中区域：角色透明部分不会误触，悬停/选中描边也按轮廓绘制。
    setShapeMode(QGraphicsPixmapItem::MaskShape);
    setTransformationMode(Qt::SmoothTransformation);
    setOffset(-pixmap.width() / 2.0, -pixmap.height());
}

QString ActorItem::actorId() const { return m_actorId; }

void ActorItem::setClassHighlighted(bool highlighted) {
    if (m_classHighlighted == highlighted) return;
    m_classHighlighted = highlighted;
    update();
}

void ActorItem::setParameterPickMode(bool enabled) {
    if (m_parameterPickMode == enabled) return;
    m_parameterPickMode = enabled;
    update();
}

void ActorItem::playAttackToward(const QPointF&) {}
void ActorItem::playHit() {}
void ActorItem::playHeal() {}

void ActorItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    QGraphicsPixmapItem::paint(painter, option, widget);

    QPainterPath spritePath = QGraphicsPixmapItem::shape();

    const bool pickingHover = m_parameterPickMode && m_hovered;

    if (m_classHighlighted) {
        painter->setPen(QPen(QColor(104, 92, 255, 230), 5, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(spritePath);
    }

    if (isSelected() && !pickingHover) {
        painter->setPen(QPen(QColor(255, 214, 92, 235), 5));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(spritePath);
    }

    if (m_hovered && !isSelected() && !m_parameterPickMode) {
        painter->setPen(QPen(QColor(120, 190, 255, 230), 4));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(spritePath);
    }

    if (pickingHover) {
        painter->setPen(QPen(QColor(15, 68, 74, 220), 9, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(spritePath);

        painter->setPen(QPen(QColor(26, 235, 202, 245), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(spritePath);

        painter->setPen(QPen(QColor(232, 255, 250, 245), 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(spritePath);
    }
}

void ActorItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    emit clicked(m_actorId);
    event->accept();
}

void ActorItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    m_hovered = true;
    update();
    emit hovered(m_actorId);
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

void ActorItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    m_hovered = false;
    update();
    emit hoverLeft(m_actorId);
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}
