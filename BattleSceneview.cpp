#include "BattleSceneView.h"
#include "SpriteUtil.h"
#include <QPainter>
#include <QBrush>
#include <QColor>
#include <QGraphicsTextItem>
#include <QDebug>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <algorithm>

BattleSceneView::BattleSceneView(QWidget* parent)
    : QGraphicsView(parent) {
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setSceneRect(0, 0, 620, 400);
    setMinimumSize(620, 400);
    setRenderHints(
        QPainter::Antialiasing |
        QPainter::SmoothPixmapTransform |
        QPainter::TextAntialiasing
        );
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(QBrush(QColor(30, 30, 42)));
    setFrameShape(QFrame::NoFrame);
}

void BattleSceneView::setLevel(GameLevel* level) {
    m_level = level;
    if (m_level) {
        connect(m_level, &GameLevel::levelChanged,
                this, &BattleSceneView::refresh);
    }
    refresh();
}

void BattleSceneView::refresh() {
    m_scene->clear();
    m_actorItems.clear();
    m_scene->setBackgroundBrush(QBrush(QColor(35, 35, 45)));
    if (!m_level) return;

    // ===== 1. 画玩家 =====
    Creature* player = m_level->creature("player");
    if (player) {
        QPixmap playerPixmap = SpriteUtil::loadSprite(
            ":/assets/player_creature.png", 180, 8);
        if (playerPixmap.isNull()) {
            playerPixmap = QPixmap(140, 180);
            playerPixmap.fill(Qt::transparent);
            QPainter painter(&playerPixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(QColor(80, 120, 255));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(20, 20, 100, 140);
            painter.end();
        }

        QPointF playerFootPos(180, 360);
        ActorItem* playerItem = new ActorItem("player", playerPixmap);
        playerItem->setPos(playerFootPos);
        if (!player->isAlive()) playerItem->setOpacity(0.35);
        m_scene->addItem(playerItem);
        m_actorItems.insert("player", playerItem);
        addHpBar(player, playerFootPos);

        connect(playerItem, &ActorItem::clicked, this,
                [this](const QString& actorId) {
                    m_selectedActorId = actorId;
                    updateSelection();
                    emit actorSelected(actorId);
                });
    }

    // ===== 2. 收集所有敌人（按 id 排序，保证 enemy1 在左、enemy2 在右） =====
    QList<Creature*> enemies;
    for (Creature* c : m_level->creatures()) {
        if (c && c->isEnemy()) enemies.append(c);
    }
    std::sort(enemies.begin(), enemies.end(),
              [](Creature* a, Creature* b) { return a->id() < b->id(); });

    // ===== 3. 按敌人数量分布站位 =====
    const int n = enemies.size();
    QList<QPointF> enemyPositions;
    if (n == 1) {
        enemyPositions.append(QPointF(470, 360));
    } else if (n == 2) {
        enemyPositions.append(QPointF(380, 360));
        enemyPositions.append(QPointF(540, 360));
    } else if (n >= 3) {
        // 3 个及以上：在 x=340~580 之间平均分布
        const int startX = 340;
        const int endX   = 580;
        for (int i = 0; i < n; ++i) {
            int x = (n == 1) ? (startX + endX) / 2
                             : startX + (endX - startX) * i / (n - 1);
            enemyPositions.append(QPointF(x, 360));
        }
    }

    // ===== 4. 画敌人 =====
    for (int i = 0; i < n; ++i) {
        Creature* enemy = enemies[i];
        const QPointF& footPos = enemyPositions[i];

        QPixmap enemyPixmap = SpriteUtil::loadSprite(
            ":/assets/enemy_bug.png", 100, 8);
        if (enemyPixmap.isNull()) {
            enemyPixmap = QPixmap(180, 120);
            enemyPixmap.fill(Qt::transparent);
            QPainter painter(&enemyPixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(QColor(180, 60, 70));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(20, 20, 140, 80);
            painter.end();
        }

        ActorItem* enemyItem = new ActorItem(enemy->id(), enemyPixmap);
        enemyItem->setPos(footPos);
        if (!enemy->isAlive()) enemyItem->setOpacity(0.35);
        m_scene->addItem(enemyItem);
        m_actorItems.insert(enemy->id(), enemyItem);
        addHpBar(enemy, footPos);

        connect(enemyItem, &ActorItem::clicked, this,
                [this](const QString& actorId) {
                    m_selectedActorId = actorId;
                    updateSelection();
                    emit actorSelected(actorId);
                });
    }

    // ===== 5. 画 Steps 文本 =====
    QString stepText = QString("Steps: %1 / %2")
                           .arg(m_level->usedSteps())
                           .arg(m_level->maxSteps());
    QGraphicsTextItem* text = m_scene->addText(stepText);
    text->setDefaultTextColor(Qt::white);
    text->setPos(35, 35);
    QFont font = text->font();
    font.setPointSize(11);
    font.setBold(true);
    text->setFont(font);

    updateSelection();
}

void BattleSceneView::setSelectedActor(const QString& actorId) {
    m_selectedActorId = actorId;
    updateSelection();
}

void BattleSceneView::updateSelection() {
    for (auto it = m_actorItems.begin(); it != m_actorItems.end(); ++it) {
        ActorItem* item = it.value();
        if (item) {
            item->setSelected(it.key() == m_selectedActorId);
            item->update();
        }
    }
}

void BattleSceneView::addHpBar(Creature* creature, const QPointF& footPos) {
    if (!creature) return;
    const qreal barWidth  = creature->isEnemy() ? 100 : 120;
    const qreal barHeight = 18;
    const qreal x = footPos.x() - barWidth / 2.0;
    const qreal y = footPos.y() + 8;

    double ratio = 0.0;
    if (creature->maxHp() > 0) {
        ratio = static_cast<double>(creature->hp()) / creature->maxHp();
    }
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    QColor hpColor(110, 219, 116);
    if (ratio <= 0.3)      hpColor = QColor(255, 107, 107);
    else if (ratio <= 0.6) hpColor = QColor(255, 211, 110);

    QGraphicsRectItem* bg = m_scene->addRect(
        x, y, barWidth, barHeight,
        QPen(QColor(143, 162, 214), 1.2),
        QBrush(QColor(29, 33, 48)));
    bg->setZValue(20);

    QGraphicsRectItem* fill = m_scene->addRect(
        x + 2, y + 2,
        (barWidth - 4) * ratio, barHeight - 4,
        Qt::NoPen,
        QBrush(hpColor));
    fill->setZValue(21);

    QString hpText = QString("%1 / %2").arg(creature->hp()).arg(creature->maxHp());
    QGraphicsSimpleTextItem* hpLabel = m_scene->addSimpleText(hpText);
    QFont hpFont("Microsoft YaHei UI", 9, QFont::Bold);
    hpLabel->setFont(hpFont);
    hpLabel->setBrush(QBrush(Qt::white));
    hpLabel->setZValue(22);
    QRectF hpRect = hpLabel->boundingRect();
    hpLabel->setPos(
        x + (barWidth - hpRect.width()) / 2.0,
        y + (barHeight - hpRect.height()) / 2.0 - 1);

    QString nameText = creature->id();
    QGraphicsSimpleTextItem* nameLabel = m_scene->addSimpleText(nameText);
    QFont nameFont("Microsoft YaHei UI", 9, QFont::Normal);
    nameLabel->setFont(nameFont);
    nameLabel->setBrush(QBrush(QColor(230, 235, 245)));
    nameLabel->setZValue(22);
    QRectF nameRect = nameLabel->boundingRect();
    nameLabel->setPos(
        x + (barWidth - nameRect.width()) / 2.0,
        y + barHeight + 4);
}
