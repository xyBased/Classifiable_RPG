#pragma once
#include <QObject>
#include <QString>
#include "GameLevel.h"

struct CommandResult {
    bool success = false;
    bool consumeStep = false;
    QString message;
    QString sourceId;     // 新增：本次指令的发出者 id
};

class CommandParser : public QObject {
    Q_OBJECT
private:
    GameLevel* m_level = nullptr;
public:
    explicit CommandParser(GameLevel* level, QObject* parent = nullptr);
    CommandResult execute(const QString& command);
};
