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
    QRegularExpression re(R"(^(\w+)\.(\w+)\((\w*)\);$)");
    QRegularExpressionMatch match = re.match(text);
    if (!match.hasMatch()) {
        result.message = "指令格式错误。示例：player.attack(enemy);";
        return result;
    }

    QString sourceId = match.captured(1);
    QString method   = match.captured(2);
    QString targetId = match.captured(3);

    Creature* source = m_level->creature(sourceId);
    Creature* target = nullptr;
    if (!targetId.isEmpty()) target = m_level->creature(targetId);

    if (!source) {
        result.message = "找不到对象：" + sourceId;
        return result;
    }
    if (!source->isAlive()) {
        result.message = sourceId + " 已经无法行动。";
        return result;
    }

    // 关键：是否允许玩家直接调用敌人对象的方法
    if (source->isEnemy()) {
        if (!m_level->allowEnemyControl()) {
            result.message = "attack 在 Enemy 类的 protected 区段，外部无法调用。";
            return result;
        }
        // 即使放开，敌人类对外只暴露了 attack（public 部分只有它）
        if (method != "attack") {
            result.message = QString("Enemy 类的 public 区段未暴露 %1。").arg(method);
            return result;
        }
    }

    // 通用规则：不能自己打自己（防止 enemy2.attack(enemy2); 这种 hack）
    if (target && source == target) {
        result.message = "一只生物不能把自己作为攻击目标。";
        return result;
    }

    result.sourceId = sourceId;

    if (method == "attack") {
        if (!target) {
            result.message = "attack 需要一个目标。";
            return result;
        }
        target->takeDamage(source->atk());
        result.success = true;
        result.consumeStep = true;
        result.message = QString("%1.attack(%2); 造成 %3 点伤害。")
                             .arg(sourceId, targetId).arg(source->atk());
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
                             .arg(sourceId, targetId).arg(damage);
        return result;
    }

    if (method == "heal") {
        source->heal(3);
        result.success = true;
        result.consumeStep = true;
        result.message = QString("%1.heal(); 恢复 3 点生命。").arg(sourceId);
        return result;
    }

    result.message = "这个方法当前不可用：" + method;
    return result;
}
