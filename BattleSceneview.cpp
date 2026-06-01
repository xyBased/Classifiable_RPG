#include "BattleSceneView.h"

#include "SpriteUtil.h"

#include <QBrush>
#include <QFrame>
#include <QGraphicsPathItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsSimpleTextItem>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QtGlobal>
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
    m_selectedActorId = actorId;
    updateSelection();
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
    if (player) {
        int targetHeight = 245;
        QPixmap playerPixmap = spriteFor(player, targetHeight);
        QPointF footPos(200, 425);

        ActorItem* item = new ActorItem("player", playerPixmap);
        item->setPos(footPos);
        item->setZValue(20);
        if (!player->isAlive()) item->setOpacity(0.35);
        m_scene->addItem(item);
        m_actorItems.insert("player", item);
        m_actorPositions.insert("player", footPos);
        addHpBar(player, footPos, 220);

        connect(item, &ActorItem::clicked, this, [this](const QString& actorId) {
            m_selectedActorId = actorId;
            updateSelection();
            emit actorSelected(actorId);
        });
    }

    QList<Creature*> enemies;
    for (Creature* c : m_level->creatures()) {
        if (c && c->isEnemy()) enemies.append(c);
    }
    std::sort(enemies.begin(), enemies.end(), [](Creature* a, Creature* b) {
        return a->id() < b->id();
    });

    const int n = enemies.size();
    QList<QPointF> positions;
    if (n == 1) {
        positions << QPointF(660, 425);
    } else {
        const int startX = (n >= 4) ? 510 : 560;
        const int endX = 790;
        for (int i = 0; i < n; ++i) {
            int x = startX + (endX - startX) * i / qMax(1, n - 1);
            positions << QPointF(x, (i % 2 == 0) ? 425 : 405);
        }
    }

    for (int i = 0; i < enemies.size(); ++i) {
        Creature* enemy = enemies[i];
        int targetHeight = 180;
        QPixmap pixmap = spriteFor(enemy, targetHeight);
        QPointF footPos = positions.value(i, QPointF(660, 425));

        ActorItem* item = new ActorItem(enemy->id(), pixmap);
        item->setPos(footPos);
        item->setZValue(20 + i);
        if (!enemy->isAlive()) item->setOpacity(0.25);
        m_scene->addItem(item);

        m_actorItems.insert(enemy->id(), item);
        m_actorPositions.insert(enemy->id(), footPos);

        addIntent(enemy, footPos, pixmap.height());
        addHpBar(enemy, footPos, qBound(110, pixmap.width(), 180));

        connect(item, &ActorItem::clicked, this, [this](const QString& actorId) {
            m_selectedActorId = actorId;
            updateSelection();
            emit actorSelected(actorId);
        });
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
    QString text = QString("关卡 %1 / %2    步数 %3 / %4")
        .arg(m_level->currentLevel())
        .arg(GameLevel::TotalLevels)
        .arg(m_level->usedSteps())
        .arg(m_level->maxSteps());

    auto* bg = addRoundedRectItem(
        m_scene,
        QRectF(560, 20, 270, 48),
        16, 16,
        QPen(QColor(125, 146, 200, 210), 2),
        QBrush(QColor(255, 255, 255, 215))
    );
    bg->setZValue(80);

    auto* label = m_scene->addText(text);
    label->setDefaultTextColor(QColor(35, 45, 78));
    label->setScale(1.05);
    label->setPos(580, 28);
    label->setZValue(81);
}

void BattleSceneView::addHpBar(Creature* creature, const QPointF& footPos, int width) {
    const int barWidth = width;
    const int barHeight = 18;
    const int x = static_cast<int>(footPos.x() - barWidth / 2);
    const int y = static_cast<int>(footPos.y() + 12);
    const double ratio = creature->maxHp() > 0 ? double(creature->hp()) / creature->maxHp() : 0.0;

    auto* back = addRoundedRectItem(
        m_scene,
        QRectF(x, y, barWidth, barHeight),
        7, 7,
        QPen(QColor(46, 53, 74), 1),
        QBrush(QColor(246, 248, 255, 230))
    );
    back->setZValue(40);

    QColor color = ratio <= 0.3 ? QColor(255, 107, 107) : (ratio <= 0.6 ? QColor(255, 205, 94) : QColor(105, 223, 124));
    auto* front = addRoundedRectItem(
        m_scene,
        QRectF(x + 1, y + 1, qMax(0, int((barWidth - 2) * ratio)), barHeight - 2),
        6, 6,
        Qt::NoPen,
        QBrush(color)
    );
    front->setZValue(41);

    auto* hpText = m_scene->addText(QString("%1 / %2").arg(creature->hp()).arg(creature->maxHp()));
    hpText->setDefaultTextColor(QColor(25, 30, 55));
    hpText->setScale(0.72);
    hpText->setPos(footPos.x() - 26, y - 2);
    hpText->setZValue(42);
}

void BattleSceneView::addIntent(Creature* creature, const QPointF& footPos, int spriteHeight) {
    if (!creature || !creature->isEnemy() || !creature->isAlive() || creature->intent() == Creature::IntentNone) return;

    const QString label = creature->intent() == Creature::IntentAttack
        ? QString("%1 %2").arg(creature->intentSymbol()).arg(creature->intentValue())
        : creature->intentSymbol();

    const int w = creature->intent() == Creature::IntentAttack ? 78 : 52;
    const QPointF p(footPos.x() - w / 2.0, footPos.y() - spriteHeight - 52);

    auto* bubble = addRoundedRectItem(
        m_scene,
        QRectF(p.x(), p.y(), w, 42),
        14, 14,
        QPen(QColor(109, 123, 171, 180), 2),
        QBrush(QColor(255, 255, 255, 228))
    );
    bubble->setZValue(70);

    auto* text = m_scene->addText(label);
    QColor color = QColor(255, 170, 60);
    if (creature->intent() == Creature::IntentHeal) color = QColor(67, 175, 99);
    if (creature->intent() == Creature::IntentBuff) color = QColor(60, 125, 235);
    if (creature->intent() == Creature::IntentDefend) color = QColor(90, 90, 140);
    text->setDefaultTextColor(color);
    text->setScale(1.15);
    text->setPos(p.x() + 10, p.y() + 4);
    text->setZValue(71);

    auto* tip = m_scene->addText(creature->intentText());
    tip->setDefaultTextColor(QColor(32, 41, 69, 200));
    tip->setScale(0.68);
    tip->setPos(p.x() - 2, p.y() + 41);
    tip->setZValue(71);
}

QPixmap BattleSceneView::spriteFor(Creature* creature, int& targetHeight) const {
    if (!creature) return QPixmap();

    QString path = ":/assets/enemy_bug.png";
    targetHeight = creature->isEnemy() ? 180 : 245;

    if (!creature->isEnemy()) {
        path = ":/assets/player_hero.png";
        targetHeight = 250;
    } else {
        const QString key = (creature->id() + " " + creature->name() + " " + creature->className()).toLower();
        if (key.contains("boss") || key.contains("king") || key.contains("door")) {
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
        it.value()->setSelected(it.key() == m_selectedActorId);
        it.value()->update();
    }
}
