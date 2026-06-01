#include "SpriteUtil.h"

#include <QColor>
#include <QDebug>
#include <QImage>
#include <QPoint>
#include <QQueue>
#include <QRect>
#include <QtGlobal>

static bool isBackgroundLike(const QRgb& pixel) {
    int a = qAlpha(pixel);
    if (a == 0) return true;

    QColor c(pixel);
    int r = c.red();
    int g = c.green();
    int b = c.blue();

    // 生成图有时带棋盘格/浅灰底。只清理与边缘连通的浅色背景，避免误抠角色内部高光。
    bool nearWhiteOrGray =
        r > 205 && g > 205 && b > 205 &&
        qAbs(r - g) < 38 &&
        qAbs(r - b) < 38 &&
        qAbs(g - b) < 38;

    return nearWhiteOrGray;
}

static void clearConnectedBackground(QImage& image) {
    if (image.isNull()) return;

    const int w = image.width();
    const int h = image.height();

    QVector<QVector<bool>> visited(h, QVector<bool>(w, false));
    QQueue<QPoint> queue;

    auto tryPush = [&](int x, int y) {
        if (x < 0 || x >= w || y < 0 || y >= h) return;
        if (visited[y][x]) return;

        if (isBackgroundLike(image.pixel(x, y))) {
            visited[y][x] = true;
            queue.enqueue(QPoint(x, y));
        }
    };

    for (int x = 0; x < w; ++x) {
        tryPush(x, 0);
        tryPush(x, h - 1);
    }
    for (int y = 0; y < h; ++y) {
        tryPush(0, y);
        tryPush(w - 1, y);
    }

    while (!queue.isEmpty()) {
        QPoint p = queue.dequeue();
        QColor c = QColor::fromRgba(image.pixel(p.x(), p.y()));
        c.setAlpha(0);
        image.setPixelColor(p.x(), p.y(), c);

        tryPush(p.x() + 1, p.y());
        tryPush(p.x() - 1, p.y());
        tryPush(p.x(), p.y() + 1);
        tryPush(p.x(), p.y() - 1);
    }
}

static QRect findContentRect(const QImage& image) {
    const int w = image.width();
    const int h = image.height();

    int minX = w, minY = h, maxX = -1, maxY = -1;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (qAlpha(image.pixel(x, y)) > 10) {
                minX = qMin(minX, x);
                minY = qMin(minY, y);
                maxX = qMax(maxX, x);
                maxY = qMax(maxY, y);
            }
        }
    }

    if (maxX < minX || maxY < minY) return QRect();
    return QRect(QPoint(minX, minY), QPoint(maxX, maxY));
}

QPixmap SpriteUtil::loadSprite(const QString& path, int targetHeight, int padding) {
    QImage image(path);
    if (image.isNull()) {
        qDebug() << "Sprite load failed:" << path;
        return QPixmap();
    }

    image = image.convertToFormat(QImage::Format_ARGB32);
    clearConnectedBackground(image);

    QRect contentRect = findContentRect(image);
    if (!contentRect.isNull()) {
        contentRect = contentRect.adjusted(-padding, -padding, padding, padding).intersected(image.rect());
        image = image.copy(contentRect);
    }

    if (targetHeight > 0 && image.height() != targetHeight) {
        image = image.scaledToHeight(targetHeight, Qt::SmoothTransformation);
    }

    return QPixmap::fromImage(image);
}
