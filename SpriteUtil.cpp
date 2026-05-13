#include "SpriteUtil.h"

#include <QImage>
#include <QColor>
#include <QQueue>
#include <QPoint>
#include <QRect>
#include <QDebug>

static bool isBackgroundLike(const QRgb& pixel) {
    int a = qAlpha(pixel);

    if (a == 0) {
        return true;
    }

    QColor c(pixel);

    int r = c.red();
    int g = c.green();
    int b = c.blue();

    // 只判断接近白色/浅灰色的背景色。
    // 注意：不要简单地把所有白色都变透明，否则白发角色会被抠坏。
    bool nearWhiteOrGray =
        r > 215 &&
        g > 215 &&
        b > 215 &&
        qAbs(r - g) < 30 &&
        qAbs(r - b) < 30 &&
        qAbs(g - b) < 30;

    return nearWhiteOrGray;
}

static void clearConnectedBackground(QImage& image) {
    if (image.isNull()) return;

    int w = image.width();
    int h = image.height();

    QVector<QVector<bool>> visited(h, QVector<bool>(w, false));
    QQueue<QPoint> queue;

    auto tryPush = [&](int x, int y) {
        if (x < 0 || x >= w || y < 0 || y >= h) return;
        if (visited[y][x]) return;

        QRgb pixel = image.pixel(x, y);

        if (isBackgroundLike(pixel)) {
            visited[y][x] = true;
            queue.enqueue(QPoint(x, y));
        }
    };

    // 从四周边缘开始清理背景，只清理和边缘连通的浅色背景。
    // 这样可以尽量避免把角色内部的白发、白衣服抠掉。
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
        int x = p.x();
        int y = p.y();

        QColor c = QColor::fromRgba(image.pixel(x, y));
        c.setAlpha(0);
        image.setPixelColor(x, y, c);

        tryPush(x + 1, y);
        tryPush(x - 1, y);
        tryPush(x, y + 1);
        tryPush(x, y - 1);
    }
}

static QRect findContentRect(const QImage& image) {
    int w = image.width();
    int h = image.height();

    int minX = w;
    int minY = h;
    int maxX = -1;
    int maxY = -1;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int alpha = qAlpha(image.pixel(x, y));

            if (alpha > 10) {
                minX = qMin(minX, x);
                minY = qMin(minY, y);
                maxX = qMax(maxX, x);
                maxY = qMax(maxY, y);
            }
        }
    }

    if (maxX < minX || maxY < minY) {
        return QRect();
    }

    return QRect(QPoint(minX, minY), QPoint(maxX, maxY));
}

QPixmap SpriteUtil::loadSprite(const QString& path, int targetHeight, int padding) {
    QImage image(path);

    if (image.isNull()) {
        qDebug() << "Sprite load failed:" << path;
        return QPixmap();
    }

    image = image.convertToFormat(QImage::Format_ARGB32);

    // 第一步：去掉边缘白色/棋盘格背景
    clearConnectedBackground(image);

    // 第二步：根据非透明区域裁剪
    QRect contentRect = findContentRect(image);

    if (!contentRect.isNull()) {
        contentRect = contentRect.adjusted(-padding, -padding, padding, padding);
        contentRect = contentRect.intersected(image.rect());
        image = image.copy(contentRect);
    }

    // 第三步：缩放到指定高度
    if (targetHeight > 0 && image.height() != targetHeight) {
        image = image.scaledToHeight(
            targetHeight,
            Qt::SmoothTransformation
            );
    }

    return QPixmap::fromImage(image);
}
