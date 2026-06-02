#pragma once

#include <QPixmap>
#include <QString>

namespace SpriteUtil {
    QPixmap loadSprite(const QString& path, int targetHeight, int padding = 8);
}
