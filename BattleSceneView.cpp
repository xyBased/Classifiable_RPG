#include "BattleSceneView.h"

#include "SpriteUtil.h"

#include <QBrush>
#include <QFont>
#include <QFrame>
#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsTextItem>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QRadialGradient>
#include <QMouseEvent>
#include <QtGlobal>
#include <QTransform>
#include <algorithm>

static QGraphicsPathItem* addRoundedRectItem(
    QGraphicsScene* scene,
    const QRectF& rect,
    qreal rx,
    qreal ry,
    const QPen& pen,
    const QBrush& brush
) {
    QPainterPath path;
    path.addRoundedRect(rect, rx, ry);
    return scene->addPath(path, pen, brush);
}

static QColor intentAccent(Creature::EnemyIntent intent) {
    switch (intent) {
    case Creature::IntentAttack: return QColor(255, 170, 60);
    case Creature::IntentDefend: return QColor(110, 180, 255);
    case Creature::IntentHeal: return QColor(93, 221, 135);
    case Creature::IntentBuff: return QColor(173, 120, 255);
    case Creature::IntentUnknown: return QColor(200, 200, 215);
    default: return QColor(185, 190, 205);
    }
}

static void addIntentBackdrop(QGraphicsScene* scene, const QRectF& rect, const QColor& accent, qreal z) {
    QRadialGradient glow(rect.center(), rect.width() * 0.9);
    QColor outer = accent; outer.setAlpha(0);
    QColor mid = accent; mid.setAlpha(32);
    glow.setColorAt(0.0, QColor(255, 255, 255, 0));
    glow.setColorAt(0.72, mid);
    glow.setColorAt(1.0, outer);
    auto* halo = scene->addEllipse(rect.adjusted(-4, -4, 4, 4), Qt::NoPen, QBrush(glow));
    halo->setZValue(z);

    auto* shadow = scene->addEllipse(rect.adjusted(1.5, 2.2, 1.5, 2.2), Qt::NoPen, QBrush(QColor(24, 32, 54, 42)));
    shadow->setZValue(z + 0.02);

    auto* disk = scene->addEllipse(
        rect,
        QPen(QColor(216, 186, 138, 210), 1.6),
        QBrush(QColor(252, 252, 252, 242))
    );
    disk->setZValue(z + 0.1);

    auto* rim = scene->addEllipse(
        rect.adjusted(1.8, 1.8, -1.8, -1.8),
        QPen(QColor(255, 255, 255, 210), 1.1),
        Qt::NoBrush
    );
    rim->setZValue(z + 0.16);

    QRadialGradient sheen(rect.center() + QPointF(-4, -5), rect.width() * 0.72);
    sheen.setColorAt(0.0, QColor(255, 255, 255, 70));
    sheen.setColorAt(0.55, QColor(255, 255, 255, 18));
    sheen.setColorAt(1.0, QColor(255, 255, 255, 0));
    auto* sheenItem = scene->addEllipse(rect.adjusted(1.2, 1.2, -1.2, -1.2), Qt::NoPen, QBrush(sheen));
    sheenItem->setZValue(z + 0.2);
}


static bool addReferenceIntentPixmap(QGraphicsScene* scene, const QRectF& rect, const QString& resourcePath, qreal z) {
    QPixmap pix(resourcePath);
    if (pix.isNull()) return false;

    QPixmap scaled = pix.scaled(
        rect.size().toSize(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    auto* item = scene->addPixmap(scaled);
    item->setPos(
        rect.center().x() - scaled.width() / 2.0,
        rect.center().y() - scaled.height() / 2.0
    );
    item->setZValue(z);
    return true;
}

static void addValueText(QGraphicsScene* scene, const QPointF& pos, const QString& text, const QColor& color, qreal z, qreal scale = 0.95) {
    auto* value = scene->addText(text);
    QFont font = value->font();
    font.setBold(true);
    value->setFont(font);
    value->setDefaultTextColor(color);
    value->setScale(scale);
    QRectF br = value->boundingRect();
    value->setPos(pos.x() - br.width() * scale / 2.0, pos.y() - br.height() * scale / 2.0);
    value->setZValue(z);
}

static void addValuePill(QGraphicsScene* scene, const QRectF& rect, const QString& text, const QColor&, qreal z) {
    auto* shadow = addRoundedRectItem(
        scene,
        rect.adjusted(1.5, 2.0, 1.5, 2.0),
        rect.height() / 2.0,
        rect.height() / 2.0,
        Qt::NoPen,
        QBrush(QColor(24, 32, 54, 36))
    );
    shadow->setZValue(z - 0.05);

    auto* pill = addRoundedRectItem(
        scene,
        rect,
        rect.height() / 2.0,
        rect.height() / 2.0,
        QPen(QColor(216, 186, 138, 210), 1.8),
        QBrush(QColor(252, 252, 252, 240))
    );
    pill->setZValue(z);

    auto* rim = addRoundedRectItem(
        scene,
        rect.adjusted(1.5, 1.5, -1.5, -1.5),
        rect.height() / 2.0 - 1.0,
        rect.height() / 2.0 - 1.0,
        QPen(QColor(255, 255, 255, 200), 1.0),
        Qt::NoBrush
    );
    rim->setZValue(z + 0.03);

    auto* shine = addRoundedRectItem(
        scene,
        QRectF(rect.left() + 4, rect.top() + 4, rect.width() - 8, 4),
        2.0,
        2.0,
        Qt::NoPen,
        QBrush(QColor(255, 255, 255, 90))
    );
    shine->setZValue(z + 0.04);

    QFont font;
    font.setFamily("Arial");
    font.setPointSize(17);
    font.setBold(true);

    auto* shadowText = new QGraphicsSimpleTextItem(text);
    shadowText->setFont(font);
    shadowText->setBrush(QBrush(QColor(255, 255, 255, 140)));
    QRectF sbr = shadowText->boundingRect();
    shadowText->setPos(rect.center().x() - sbr.width() / 2.0, rect.center().y() - sbr.height() / 2.0 - 0.6);
    shadowText->setZValue(z + 0.05);
    scene->addItem(shadowText);

    auto* value = new QGraphicsSimpleTextItem(text);
    value->setFont(font);
    value->setBrush(QBrush(QColor(23, 37, 78)));
    QRectF br = value->boundingRect();
    value->setPos(rect.center().x() - br.width() / 2.0, rect.center().y() - br.height() / 2.0);
    value->setZValue(z + 0.1);
    scene->addItem(value);
}

static void addShieldIcon(QGraphicsScene* scene, const QRectF& rect, const QColor&, qreal z) {
    const QPointF c = rect.center();

    QPainterPath outer;
    outer.moveTo(c.x(), rect.top() + 1.2);
    outer.lineTo(rect.right() - 3.0, rect.top() + 7.5);
    outer.lineTo(rect.right() - 4.2, c.y() + 5.0);
    outer.quadTo(c.x(), rect.bottom() - 1.2, rect.left() + 4.2, c.y() + 5.0);
    outer.lineTo(rect.left() + 3.0, rect.top() + 7.5);
    outer.closeSubpath();

    auto* shadow = scene->addPath(
        outer.translated(1.0, 1.3),
        Qt::NoPen,
        QBrush(QColor(18, 30, 56, 55))
    );
    shadow->setZValue(z - 0.06);

    QLinearGradient fillGrad(rect.left(), rect.top(), rect.left(), rect.bottom());
    fillGrad.setColorAt(0.0, QColor(92, 171, 255));
    fillGrad.setColorAt(0.50, QColor(59, 134, 235));
    fillGrad.setColorAt(1.0, QColor(38, 92, 203));

    auto* fill = scene->addPath(
        outer,
        QPen(QColor(28, 54, 119, 245), 2.1),
        QBrush(fillGrad)
    );
    fill->setZValue(z);

    QPainterPath inner;
    inner.moveTo(c.x(), rect.top() + 4.8);
    inner.lineTo(rect.right() - 7.5, rect.top() + 10.0);
    inner.lineTo(rect.right() - 8.2, c.y() + 2.8);
    inner.quadTo(c.x(), rect.bottom() - 5.2, rect.left() + 8.2, c.y() + 2.8);
    inner.lineTo(rect.left() + 7.5, rect.top() + 10.0);
    inner.closeSubpath();

    auto* innerBorder = scene->addPath(inner, QPen(QColor(246, 251, 255, 230), 1.45), Qt::NoBrush);
    innerBorder->setZValue(z + 0.05);

    QPainterPath crest;
    crest.moveTo(c.x(), rect.top() + 8.4);
    crest.lineTo(c.x(), rect.bottom() - 8.0);
    crest.moveTo(rect.left() + 9.0, c.y() - 0.8);
    crest.lineTo(rect.right() - 9.0, c.y() - 0.8);

    auto* cross = scene->addPath(
        crest,
        QPen(QColor(248, 252, 255, 245), 2.7, Qt::SolidLine, Qt::RoundCap),
        Qt::NoBrush
    );
    cross->setZValue(z + 0.1);
}

static void addHealIcon(QGraphicsScene* scene, const QRectF& rect, const QColor& color, qreal z) {
    const QPointF c = rect.center();
    auto pen = QPen(color, 4.6, Qt::SolidLine, Qt::RoundCap);
    auto* v = scene->addLine(c.x(), rect.top() + 8, c.x(), rect.bottom() - 8, pen);
    auto* h = scene->addLine(rect.left() + 8, c.y(), rect.right() - 8, c.y(), pen);
    v->setZValue(z);
    h->setZValue(z);
}

static void addBuffIcon(QGraphicsScene* scene, const QRectF& rect, const QColor& color, qreal z) {
    QPainterPath arrow;
    const QPointF c = rect.center();
    arrow.moveTo(c.x(), rect.top() + 7);
    arrow.lineTo(rect.right() - 8, rect.center().y());
    arrow.lineTo(c.x() + 5, rect.center().y());
    arrow.lineTo(c.x() + 5, rect.bottom() - 8);
    arrow.lineTo(c.x() - 5, rect.bottom() - 8);
    arrow.lineTo(c.x() - 5, rect.center().y());
    arrow.lineTo(rect.left() + 8, rect.center().y());
    arrow.closeSubpath();
    auto* item = scene->addPath(arrow, QPen(color.lighter(125), 2), QBrush(color));
    item->setZValue(z);

    for (QPointF s : {QPointF(rect.right() - 6, rect.top() + 8), QPointF(rect.left() + 9, rect.top() + 10)}) {
        auto* l1 = scene->addLine(s.x()-3, s.y(), s.x()+3, s.y(), QPen(QColor(255,255,255,155), 1.4, Qt::SolidLine, Qt::RoundCap));
        auto* l2 = scene->addLine(s.x(), s.y()-3, s.x(), s.y()+3, QPen(QColor(255,255,255,155), 1.4, Qt::SolidLine, Qt::RoundCap));
        l1->setZValue(z + 0.1);
        l2->setZValue(z + 0.1);
    }
}

static void addUnknownIcon(QGraphicsScene* scene, const QRectF& rect, const QColor& color, qreal z) {
    QPainterPath diamond;
    const QPointF c = rect.center();
    diamond.moveTo(c.x(), rect.top() + 7);
    diamond.lineTo(rect.right() - 7, c.y());
    diamond.lineTo(c.x(), rect.bottom() - 7);
    diamond.lineTo(rect.left() + 7, c.y());
    diamond.closeSubpath();
    auto* item = scene->addPath(diamond, QPen(color.lighter(125), 2), QBrush(QColor(120, 125, 145, 205)));
    item->setZValue(z);
    addValueText(scene, c + QPointF(0, -1), "?", color, z + 0.1, 0.88);
}

static void addSwordShape(QGraphicsScene* scene, const QPointF& center, qreal scale, qreal angleDeg, const QColor&, qreal z) {
    QTransform t;
    t.translate(center.x(), center.y());
    t.rotate(angleDeg);
    t.scale(scale, scale);

    QPainterPath blade;
    blade.moveTo(0, -18.5);
    blade.lineTo(-4.7, -5.2);
    blade.lineTo(-2.1, 7.4);
    blade.lineTo(2.1, 7.4);
    blade.lineTo(4.7, -5.2);
    blade.closeSubpath();

    auto* bladeShadow = scene->addPath(
        t.map(blade),
        QPen(QColor(28, 37, 63, 110), 1.0),
        QBrush(QColor(28, 37, 63, 46))
    );
    bladeShadow->setPos(0.9, 1.1);
    bladeShadow->setZValue(z - 0.04);

    auto* bladeItem = scene->addPath(
        t.map(blade),
        QPen(QColor(76, 90, 127, 235), 1.55),
        QBrush(QColor(238, 242, 250, 252))
    );
    bladeItem->setZValue(z);

    QPainterPath ridge;
    ridge.moveTo(0, -13.5);
    ridge.lineTo(0, 6.5);
    auto* ridgeItem = scene->addPath(
        t.map(ridge),
        QPen(QColor(150, 160, 188, 180), 1.0, Qt::SolidLine, Qt::RoundCap),
        Qt::NoBrush
    );
    ridgeItem->setZValue(z + 0.05);

    const QPointF g1 = t.map(QPointF(-7.6, 8.2));
    const QPointF g2 = t.map(QPointF(7.6, 8.2));
    auto* guard = scene->addLine(
        QLineF(g1, g2),
        QPen(QColor(223, 164, 60), 3.6, Qt::SolidLine, Qt::RoundCap)
    );
    guard->setZValue(z + 0.10);

    const QPointF h1 = t.map(QPointF(0, 8.7));
    const QPointF h2 = t.map(QPointF(0, 17.0));
    auto* handle = scene->addLine(
        QLineF(h1, h2),
        QPen(QColor(117, 73, 40), 3.9, Qt::SolidLine, Qt::RoundCap)
    );
    handle->setZValue(z + 0.12);

    const QPointF p = t.map(QPointF(0, 18.3));
    auto* pommel = scene->addEllipse(
        QRectF(p.x() - 2.25, p.y() - 2.25, 4.5, 4.5),
        QPen(QColor(138, 95, 40, 145), 0.6),
        QBrush(QColor(244, 181, 73))
    );
    pommel->setZValue(z + 0.14);
}

static void addAttackIcon(QGraphicsScene* scene, const QRectF& rect, const QColor& color, qreal z) {
    const QPointF c = rect.center();
    const qreal scale = qMin(rect.width(), rect.height()) / 29.0;

    addSwordShape(scene, c + QPointF(-3.0, 0.8), scale, -39.0, color, z);
    addSwordShape(scene, c + QPointF(3.0, 0.8), scale, 39.0, color, z + 0.18);
}

static bool canPickAliveActor(GameLevel* level, const QString& actorId) {
    Creature* creature = level ? level->creature(actorId) : nullptr;
    return creature && creature->isAlive();
}

BattleSceneView::BattleSceneView(QWidget* parent)
    : QGraphicsView(parent) {
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setSceneRect(0, 0, 860, 500);
    setMinimumSize(860, 500);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setBackgroundBrush(QBrush(QColor(235, 241, 255)));
}

void BattleSceneView::setLevel(GameLevel* level) {
    if (m_level) {
        disconnect(m_level, &GameLevel::levelChanged, this, &BattleSceneView::refresh);
    }
    m_level = level;
    if (m_level) {
        connect(m_level, &GameLevel::levelChanged, this, &BattleSceneView::refresh);
    }
    refresh();
}

void BattleSceneView::setSelectedActor(const QString& actorId) {
    m_classHighlightName.clear();
    m_selectedActorId = actorId;
    updateSelection();
}

void BattleSceneView::setClassHighlight(const QString& className) {
    m_selectedActorId.clear();
    m_classHighlightName = className;
    updateSelection();
}

void BattleSceneView::clearClassHighlight() {
    if (m_classHighlightName.isEmpty()) return;
    m_classHighlightName.clear();
    updateSelection();
}

void BattleSceneView::setParameterPickMode(bool enabled) {
    if (m_parameterPickMode == enabled) return;
    m_parameterPickMode = enabled;
    updateSelection();
}

bool BattleSceneView::parameterPickMode() const {
    return m_parameterPickMode;
}

void BattleSceneView::mousePressEvent(QMouseEvent* event) {
    if (m_parameterPickMode && event && m_scene) {
        const QPointF scenePos = mapToScene(event->pos());
        const QList<QGraphicsItem*> hitItems = m_scene->items(scenePos);
        for (QGraphicsItem* graphicsItem : hitItems) {
            QGraphicsItem* current = graphicsItem;
            while (current) {
                if (ActorItem* actor = dynamic_cast<ActorItem*>(current)) {
                    if (canPickAliveActor(m_level, actor->actorId())) {
                        emit actorParameterPicked(actor->actorId());
                    } else {
                        emit actorParameterPickRejected(actor->actorId());
                    }
                    event->accept();
                    return;
                }
                current = current->parentItem();
            }
        }

        for (auto it = m_actorItems.begin(); it != m_actorItems.end(); ++it) {
            ActorItem* actor = it.value();
            if (!actor) continue;
            const QPointF localPos = actor->mapFromScene(scenePos);
            if (actor->boundingRect().adjusted(-10, -10, 10, 10).contains(localPos)) {
                if (canPickAliveActor(m_level, actor->actorId())) {
                    emit actorParameterPicked(actor->actorId());
                } else {
                    emit actorParameterPickRejected(actor->actorId());
                }
                event->accept();
                return;
            }
        }

        emit actorParameterPickCanceled();
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void BattleSceneView::playAttack(const QString&, const QString&) {}
void BattleSceneView::playHit(const QString&) {}
void BattleSceneView::playHeal(const QString&) {}

void BattleSceneView::refresh() {
    m_scene->clear();
    m_actorItems.clear();
    m_actorPositions.clear();

    addBackground();
    if (!m_level) return;
    addHud();

    Creature* player = m_level->creature("player");
    QPointF playerFootPos(200, 425);
    if (player) {
        int targetHeight = 245;
        QPixmap playerPixmap = spriteFor(player, targetHeight);
        QPointF footPos = playerFootPos;

        ActorItem* item = new ActorItem("player", playerPixmap);
        item->setParameterPickMode(m_parameterPickMode);
        item->setPos(footPos);
        item->setZValue(20);
        if (!player->isAlive()) item->setOpacity(0.35);
        m_scene->addItem(item);
        m_actorItems.insert("player", item);
        m_actorPositions.insert("player", footPos);
        addHpBar(player, footPos, 220);

        connect(item, &ActorItem::clicked, this, [this](const QString& actorId) {
            if (m_parameterPickMode) {
                if (canPickAliveActor(m_level, actorId)) {
                    updateSelection();
                    emit actorParameterPicked(actorId);
                } else {
                    emit actorParameterPickRejected(actorId);
                }
                return;
            }
            m_classHighlightName.clear();
            m_selectedActorId = actorId;
            updateSelection();
            emit actorSelected(actorId);
        });
        connect(item, &ActorItem::hovered, this, &BattleSceneView::actorHovered);
        connect(item, &ActorItem::hoverLeft, this, &BattleSceneView::actorHoverLeft);
    }

    QList<Creature*> allies;
    for (Creature* c : m_level->creatures()) {
        if (c && !c->isEnemy() && c->id() != "player") allies.append(c);
    }
    std::sort(allies.begin(), allies.end(), [](Creature* a, Creature* b) {
        auto orderValue = [](Creature* c) {
            if (!c) return 999;
            if (c->id().startsWith("bomb")) return c->id().mid(4).toInt();
            return 500;
        };
        const int oa = orderValue(a);
        const int ob = orderValue(b);
        if (oa != ob) return oa < ob;
        return a->id() < b->id();
    });

    if (!allies.isEmpty()) {
        const qreal spacing = 128.0;
        const qreal y = playerFootPos.y() - 238.0;
        const qreal startX = playerFootPos.x() - spacing * (allies.size() - 1) / 2.0;
        for (int i = 0; i < allies.size(); ++i) {
            Creature* ally = allies[i];
            int th = 58;
            QPixmap pm = spriteFor(ally, th);
            QPointF footPos(startX + spacing * i, y);

            ActorItem* item = new ActorItem(ally->id(), pm);
            item->setParameterPickMode(m_parameterPickMode);
            item->setPos(footPos);
            item->setZValue(28 + i);
            if (!ally->isAlive()) item->setOpacity(0.25);
            m_scene->addItem(item);

            m_actorItems.insert(ally->id(), item);
            m_actorPositions.insert(ally->id(), footPos);
            addIntent(ally, footPos, pm.height());

            connect(item, &ActorItem::clicked, this, [this](const QString& actorId) {
                if (m_parameterPickMode) {
                    if (canPickAliveActor(m_level, actorId)) {
                        updateSelection();
                        emit actorParameterPicked(actorId);
                    } else {
                        emit actorParameterPickRejected(actorId);
                    }
                    return;
                }
                m_classHighlightName.clear();
                m_selectedActorId = actorId;
                updateSelection();
                emit actorSelected(actorId);
            });
            connect(item, &ActorItem::hovered, this, &BattleSceneView::actorHovered);
            connect(item, &ActorItem::hoverLeft, this, &BattleSceneView::actorHoverLeft);
        }
    }

    QList<Creature*> enemies;
    for (Creature* c : m_level->creatures()) {
        if (c && c->isEnemy()) enemies.append(c);
    }
    const int currentLevelForOrder = m_level ? m_level->currentLevel() : 0;
    std::sort(enemies.begin(), enemies.end(), [currentLevelForOrder](Creature* a, Creature* b) {
        if (currentLevelForOrder == 2) {
            auto level2VisualOrder = [](Creature* c) {
                if (!c) return 99;
                if (c->id() == "goblin") return 0;
                if (c->id() == "darkknight") return 1;
                return 50;
            };
            const int oa = level2VisualOrder(a);
            const int ob = level2VisualOrder(b);
            if (oa != ob) return oa < ob;
        }
        return a->id() < b->id();
    });

    const int n = enemies.size();

    // 预先加载所有敌人贴图，依据它们的实际宽度从左到右排开，保证彼此不重叠
    QList<QPixmap> pixmaps;
    qreal sumWidth = 0;
    for (Creature* e : enemies) {
        int th = 180;
        QPixmap pm = spriteFor(e, th);
        pixmaps << pm;
        sumWidth += pm.width();
    }

    const qreal areaLeft = 350.0;
    const qreal areaRight = 845.0;
    const qreal areaWidth = areaRight - areaLeft;
    const qreal gap = 24.0;
    const qreal baseFootY = 430.0;

    // 若所有贴图加间距后超出可用区域，则整体等比例缩小，从根源上消除重叠
    qreal needed = sumWidth + gap * qMax(0, n - 1);
    if (n > 1 && needed > areaWidth) {
        const qreal scale = areaWidth / needed;
        sumWidth = 0;
        for (int i = 0; i < pixmaps.size(); ++i) {
            QSize newSize = pixmaps[i].size() * scale;
            if (newSize.width() > 0 && newSize.height() > 0) {
                pixmaps[i] = pixmaps[i].scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            sumWidth += pixmaps[i].width();
        }
        needed = sumWidth + gap * qMax(0, n - 1);
    }

    QList<QPointF> positions;
    if (n == 1) {
        positions << QPointF(620, baseFootY);
    } else {
        // 把整组敌人在可用区域内居中，逐个贴图占据 [cursor, cursor + w]，互不重叠
        qreal cursor = areaLeft + qMax(0.0, (areaWidth - needed) / 2.0);
        for (int i = 0; i < n; ++i) {
            const qreal w = pixmaps.value(i).width();
            const qreal cx = cursor + w / 2.0;
            // 轻微高低错落以增加层次感；水平方向已完全分开，不会造成重叠
            const qreal y = baseFootY + ((i % 2 == 0) ? 0.0 : -16.0);
            positions << QPointF(cx, y);
            cursor += w + gap;
        }
    }

    for (int i = 0; i < enemies.size(); ++i) {
        Creature* enemy = enemies[i];
        QPixmap pixmap = pixmaps.value(i);
        QPointF footPos = positions.value(i, QPointF(620, baseFootY));

        ActorItem* item = new ActorItem(enemy->id(), pixmap);
        item->setParameterPickMode(m_parameterPickMode);
        item->setPos(footPos);
        item->setZValue(20 + i);
        if (!enemy->isAlive()) item->setOpacity(0.25);
        m_scene->addItem(item);

        m_actorItems.insert(enemy->id(), item);
        m_actorPositions.insert(enemy->id(), footPos);

        addIntent(enemy, footPos, pixmap.height());
        addHpBar(enemy, footPos, qBound(96, pixmap.width(), 170));

        connect(item, &ActorItem::clicked, this, [this](const QString& actorId) {
            if (m_parameterPickMode) {
                if (canPickAliveActor(m_level, actorId)) {
                    updateSelection();
                    emit actorParameterPicked(actorId);
                } else {
                    emit actorParameterPickRejected(actorId);
                }
                return;
            }
            m_classHighlightName.clear();
            m_selectedActorId = actorId;
            updateSelection();
            emit actorSelected(actorId);
        });
        connect(item, &ActorItem::hovered, this, &BattleSceneView::actorHovered);
        connect(item, &ActorItem::hoverLeft, this, &BattleSceneView::actorHoverLeft);
    }

    updateSelection();
}

void BattleSceneView::addBackground() {
    QPixmap bg(":/assets/background_spire.png");
    if (!bg.isNull()) {
        QPixmap scaled = bg.scaled(sceneRect().size().toSize(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        auto* pix = m_scene->addPixmap(scaled);
        pix->setPos((sceneRect().width() - scaled.width()) / 2.0, (sceneRect().height() - scaled.height()) / 2.0);
        pix->setZValue(-100);
    }

    QLinearGradient topShade(0, 0, 0, sceneRect().height());
    topShade.setColorAt(0, QColor(255, 255, 255, 0));
    topShade.setColorAt(1, QColor(255, 255, 255, 30));
    auto* shade = m_scene->addRect(sceneRect(), Qt::NoPen, QBrush(topShade));
    shade->setZValue(-90);
}

void BattleSceneView::addHud() {
    QString text = QString("步数 %1 / %2")
        .arg(m_level->usedSteps())
        .arg(m_level->maxSteps());

    auto* bg = addRoundedRectItem(
        m_scene,
        QRectF(640, 20, 150, 48),
        16, 16,
        QPen(QColor(125, 146, 200, 210), 2),
        QBrush(QColor(255, 255, 255, 215))
    );
    bg->setZValue(80);

    auto* label = m_scene->addText(text);
    label->setDefaultTextColor(QColor(35, 45, 78));
    label->setScale(1.05);
    label->setPos(660, 28);
    label->setZValue(81);
}

void BattleSceneView::addHpBar(Creature* creature, const QPointF& footPos, int width) {
    const int barWidth = qMax(width, 128);
    const int barHeight = 24;
    const int x = static_cast<int>(footPos.x() - barWidth / 2);
    const int y = static_cast<int>(footPos.y() + 14);
    const double ratio = qBound(0.0, creature->maxHp() > 0 ? double(creature->hp()) / creature->maxHp() : 0.0, 1.0);
    const QRectF outerRect(x, y, barWidth, barHeight);

    auto* shadow = addRoundedRectItem(
        m_scene,
        outerRect.adjusted(1.5, 2.0, 1.5, 2.0),
        10, 10,
        Qt::NoPen,
        QBrush(QColor(25, 32, 54, 38))
    );
    shadow->setZValue(39);

    QLinearGradient backGrad(x, y, x, y + barHeight);
    backGrad.setColorAt(0.0, QColor(252, 251, 248, 228));
    backGrad.setColorAt(1.0, QColor(240, 244, 252, 220));

    auto* back = addRoundedRectItem(
        m_scene,
        outerRect,
        10, 10,
        QPen(QColor(110, 126, 166, 150), 1.6),
        QBrush(backGrad)
    );
    back->setZValue(40);

    QColor color = ratio <= 0.3 ? QColor(244, 107, 116) : (ratio <= 0.6 ? QColor(244, 197, 94) : QColor(104, 218, 130));
    const int innerWidth = qMax(0, int((barWidth - 6) * ratio));
    if (innerWidth > 0) {
        QLinearGradient fillGrad(x + 3, y + 3, x + 3, y + barHeight - 3);
        fillGrad.setColorAt(0.0, color.lighter(122));
        fillGrad.setColorAt(1.0, color.darker(108));
        auto* front = addRoundedRectItem(
            m_scene,
            QRectF(x + 3, y + 3, innerWidth, barHeight - 6),
            7, 7,
            Qt::NoPen,
            QBrush(fillGrad)
        );
        front->setZValue(41);
    }

    auto* shine = addRoundedRectItem(
        m_scene,
        QRectF(x + 5, y + 4, barWidth - 10, 4),
        2.5, 2.5,
        Qt::NoPen,
        QBrush(QColor(255, 255, 255, 60))
    );
    shine->setZValue(41.2);

    const QString hpString = QString("%1 / %2").arg(creature->hp()).arg(creature->maxHp());
    QFont font;
    font.setFamily("Arial");
    font.setPointSize(12);
    font.setBold(true);

    auto* shadowText = new QGraphicsSimpleTextItem(hpString);
    shadowText->setFont(font);
    shadowText->setBrush(QBrush(QColor(255, 255, 255, 190)));
    QRectF sbr = shadowText->boundingRect();
    shadowText->setPos(outerRect.center().x() - sbr.width() / 2.0, outerRect.center().y() - sbr.height() / 2.0 - 0.8);
    shadowText->setZValue(42);
    m_scene->addItem(shadowText);

    auto* hpText = new QGraphicsSimpleTextItem(hpString);
    hpText->setFont(font);
    hpText->setBrush(QBrush(QColor(0, 0, 0)));
    QRectF br = hpText->boundingRect();
    hpText->setPos(outerRect.center().x() - br.width() / 2.0, outerRect.center().y() - br.height() / 2.0);
    hpText->setZValue(43);
    m_scene->addItem(hpText);
}


void BattleSceneView::addIntent(Creature* creature, const QPointF& footPos, int spriteHeight) {
    if (!creature || creature->id() == "player" || !creature->isAlive() || creature->intent() == Creature::IntentNone) return;

    const QColor accent = intentAccent(creature->intent());
    const bool isAttack = creature->intent() == Creature::IntentAttack;
    const bool isDefend = creature->intent() == Creature::IntentDefend;
    const bool showValue = creature->intent() == Creature::IntentAttack
        || creature->intent() == Creature::IntentHeal
        || (creature->intent() == Creature::IntentUnknown && creature->intentValue() > 0);

    const int badgeSize = 44;
    const int valueHeight = 31;
    const int gap = 6;

    int valueWidth = 0;
    if (showValue) {
        valueWidth = (creature->intentValue() >= 10) ? 54 : 49;
    }

    const int totalWidth = badgeSize + (showValue ? valueWidth + gap : 0);
    const qreal baseX = footPos.x() - totalWidth / 2.0;
    const qreal y = footPos.y() - spriteHeight - 44;

    QRectF badgeRect(baseX, y, badgeSize, badgeSize);

    const QString functionName = creature->intentFunctionName();
    if (!functionName.isEmpty()) {
        auto* fnText = new QGraphicsSimpleTextItem(functionName);
        QFont fnFont;
        fnFont.setFamily("Consolas");
        fnFont.setPointSize(13);
        fnFont.setBold(true);
        fnText->setFont(fnFont);
        fnText->setBrush(QBrush(QColor(42, 56, 104)));
        QRectF fnBr = fnText->boundingRect();
        fnText->setPos(badgeRect.center().x() - fnBr.width() / 2.0, badgeRect.top() - fnBr.height() - 4.0);
        fnText->setZValue(73);
        m_scene->addItem(fnText);
    }

    bool badgeDrawnFromReference = false;
    if (isAttack) {
        badgeDrawnFromReference = addReferenceIntentPixmap(
            m_scene,
            badgeRect,
            ":/assets/intent_ref_attack_badge.png",
            70
        );
    } else if (isDefend) {
        badgeDrawnFromReference = addReferenceIntentPixmap(
            m_scene,
            badgeRect,
            ":/assets/intent_ref_defend_badge.png",
            70
        );
    }

    if (!badgeDrawnFromReference) {
        addIntentBackdrop(m_scene, badgeRect, accent, 70);

        switch (creature->intent()) {
        case Creature::IntentAttack:
            addAttackIcon(m_scene, badgeRect.adjusted(2.6, 2.6, -2.6, -2.6), accent, 71);
            break;
        case Creature::IntentDefend:
            addShieldIcon(m_scene, badgeRect.adjusted(3.6, 3.0, -3.6, -3.0), accent, 71);
            break;
        case Creature::IntentHeal:
            addHealIcon(m_scene, badgeRect.adjusted(4.8, 4.8, -4.8, -4.8), accent, 71);
            break;
        case Creature::IntentBuff:
            addBuffIcon(m_scene, badgeRect.adjusted(4.8, 4.2, -4.8, -4.2), accent, 71);
            break;
        case Creature::IntentUnknown:
            addUnknownIcon(m_scene, badgeRect.adjusted(4.8, 4.2, -4.8, -4.2), accent, 71);
            break;
        default:
            break;
        }
    }

    if (showValue) {
        QRectF valueRect(
            badgeRect.right() + gap,
            badgeRect.center().y() - valueHeight / 2.0,
            valueWidth,
            valueHeight
        );
        addValuePill(m_scene, valueRect, QString::number(creature->intentValue()), accent, 72);
    }
}

QPixmap BattleSceneView::spriteFor(Creature* creature, int& targetHeight) const {
    if (!creature) return QPixmap();

    QString path = ":/assets/enemy_bug.png";
    targetHeight = creature->isEnemy() ? 180 : 245;
    const QString key = (creature->id() + " " + creature->name() + " " + creature->className()).toLower();

    if (key.contains("bomb")) {
        path = ":/assets/player_bomb.png";
        targetHeight = 58;
    } else if (!creature->isEnemy()) {
        path = ":/assets/player_hero.png";
        targetHeight = 250;
    } else {
        if (key.contains("boss") || key.contains("king") || key.contains("door")
            || key.contains("darkknight") || key.contains("elite")) {
            path = ":/assets/enemy_boss.png";
            targetHeight = 235;
        } else if (key.contains("goblin") || key.contains("mimic")) {
            path = ":/assets/enemy_goblin.png";
            targetHeight = 190;
        } else {
            path = ":/assets/enemy_bug.png";
            targetHeight = 170;
        }
    }

    return SpriteUtil::loadSprite(path, targetHeight, 12);
}

void BattleSceneView::updateSelection() {
    for (auto it = m_actorItems.begin(); it != m_actorItems.end(); ++it) {
        Creature* creature = m_level ? m_level->creature(it.key()) : nullptr;
        const bool selected = !m_selectedActorId.isEmpty() && it.key() == m_selectedActorId;
        const bool classHighlighted = creature && !m_classHighlightName.isEmpty()
            && (m_classHighlightName == "Creature" || creature->className() == m_classHighlightName);
        it.value()->setSelected(selected);
        it.value()->setClassHighlighted(classHighlighted);
        it.value()->setParameterPickMode(m_parameterPickMode && creature && creature->isAlive());
        it.value()->update();
    }
}
