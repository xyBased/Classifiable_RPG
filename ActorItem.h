#pragma once

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QString>

class ActorItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

private:
    QString m_actorId;
    bool m_hovered = false;

public:
    ActorItem(const QString& actorId, const QPixmap& pixmap);

    QString actorId() const;

signals:
    void clicked(const QString& actorId);

protected:
    void paint(
        QPainter* painter,
        const QStyleOptionGraphicsItem* option,
        QWidget* widget
        ) override;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
};
