#include "ActorItem.h"

#include <QPainter>
#include <QPen>
#include <QCursor>
#include <QGraphicsScene>

ActorItem::ActorItem(const QString& actorId, const QPixmap& pixmap)
    : QObject(),
    QGraphicsPixmapItem(pixmap),
    m_actorId(actorId) {

    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setCursor(Qt::PointingHandCursor);

    // 关键：根据图片透明区域生成 shape()
    setShapeMode(QGraphicsPixmapItem::MaskShape);
    setTransformationMode(Qt::SmoothTransformation);

    // 把脚底中心作为定位点
    setOffset(-pixmap.width() / 2.0, -pixmap.height());
}

QString ActorItem::actorId() const {
    return m_actorId;
}

void ActorItem::paint(
    QPainter* painter,
    const QStyleOptionGraphicsItem* option,
    QWidget* widget
    ) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 先正常绘制图片
    QGraphicsPixmapItem::paint(painter, option, widget);

    // 使用精灵自身轮廓
    QPainterPath spritePath = QGraphicsPixmapItem::shape();

    // 鼠标悬停：轻微蓝色描边
    if (m_hovered && !isSelected()) {
        QPen hoverPen(QColor(120, 200, 255, 140), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(hoverPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(spritePath);
    }

    // 选中：柔和金色轮廓
    if (isSelected()) {
        // 外层柔光
        QPen glowPen(QColor(255, 215, 90, 65), 6.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(glowPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(spritePath);

        // 主描边
        QPen mainPen(QColor(255, 220, 120, 180), 2.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(mainPen);
        painter->drawPath(spritePath);
    }
}

void ActorItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (scene()) {
        scene()->clearSelection();
    }

    setSelected(true);
    emit clicked(m_actorId);

    event->accept();
}

void ActorItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    m_hovered = true;
    update();
}

void ActorItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    m_hovered = false;
    update();
}
