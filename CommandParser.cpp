#include "CommandParser.h"

#include <QRegularExpression>

CommandParser::CommandParser(GameLevel* level, QObject* parent)
    : QObject(parent), m_level(level) {}

CommandResult CommandParser::execute(const QString& command) {
    CommandResult result;

    if (!m_level) {
        result.message = "关卡不存在。";
        return result;
    }

    if (m_level->isWin()) {
        result.message = "敌人已经被击败。";
        return result;
    }

    if (m_level->isLose()) {
        result.message = "步数已经用完。";
        return result;
    }

    QString text = command.trimmed();

    QRegularExpression re(
        R"(^(\w+)\.(\w+)\((\w*)\);$)"
        );

    QRegularExpressionMatch match = re.match(text);

    if (!match.hasMatch()) {
        result.message = "指令格式错误。示例：player.attack(enemy);";
        return result;
    }

    QString sourceId = match.captured(1);
    QString method = match.captured(2);
    QString targetId = match.captured(3);

    Creature* source = m_level->creature(sourceId);
    Creature* target = nullptr;

    if (!targetId.isEmpty()) {
        target = m_level->creature(targetId);
    }

    if (!source) {
        result.message = "找不到对象：" + sourceId;
        return result;
    }

    if (!source->isAlive()) {
        result.message = sourceId + " 已经无法行动。";
        return result;
    }

    // 规则：玩家只能控制 player，不能直接控制 enemy
    if (sourceId != "player") {
        result.message = "你不能直接控制敌人对象。";
        return result;
    }

    if (method == "attack") {
        if (!target) {
            result.message = "attack 需要一个目标。";
            return result;
        }

        target->takeDamage(source->atk());

        result.success = true;
        result.consumeStep = true;
        result.message = QString("%1.attack(%2); 造成 %3 点伤害。")
                             .arg(sourceId, targetId)
                             .arg(source->atk());
        return result;
    }

    if (method == "powerAttack") {
        if (!target) {
            result.message = "powerAttack 需要一个目标。";
            return result;
        }

        int damage = source->atk() * 2;
        target->takeDamage(damage);

        result.success = true;
        result.consumeStep = true;
        result.message = QString("%1.powerAttack(%2); 造成 %3 点伤害。")
                             .arg(sourceId, targetId)
                             .arg(damage);
        return result;
    }

    if (method == "heal") {
        source->heal(3);

        result.success = true;
        result.consumeStep = true;
        result.message = "player.heal(); 恢复 3 点生命。";
        return result;
    }

    result.message = "这个方法当前不可用：" + method;
    return result;
}
