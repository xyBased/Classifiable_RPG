#pragma once

#include <QObject>
#include <QString>

#include "GameLevel.h"

struct CommandResult {
    bool success = false;
    bool consumeStep = false;
    QString message;
    QString sourceId;
    QString targetId;
    QString effect;
};

class CommandParser : public QObject {
    Q_OBJECT
private:
    GameLevel* m_level = nullptr;

public:
    explicit CommandParser(GameLevel* level, QObject* parent = nullptr);
    CommandResult execute(const QString& command);
};
