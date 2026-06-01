#include "CommandParser.h"

#include <QRegularExpression>
#include <QStringList>
#include <algorithm>

static QString compactCommand(QString s) {
    s.remove(QRegularExpression("\\s+"));
    return s;
}

static Creature* obj(GameLevel* level, const QString& id) {
    return level ? level->creature(id.trimmed()) : nullptr;
}

static QString hpText(Creature* c) {
    if (!c) return {};
    return QString("%1 HP: %2 / %3").arg(c->id()).arg(c->hp()).arg(c->maxHp());
}

static QString joinStatus(Creature* a, Creature* b = nullptr) {
    QStringList parts;
    if (a) parts << hpText(a);
    if (b) parts << hpText(b);
    return parts.join("\n");
}

static CommandResult note(const QString& message, const QString& sourceId = {}, const QString& targetId = {}) {
    CommandResult r;
    r.success = false;
    r.consumeStep = false;
    r.message = message;
    r.sourceId = sourceId;
    r.targetId = targetId;
    r.effect = "none";
    return r;
}

static CommandResult ok(
    const QString& message,
    const QString& sourceId,
    const QString& targetId = {},
    const QString& effect = "none",
    bool consumeStep = true
) {
    CommandResult r;
    r.success = true;
    r.consumeStep = consumeStep;
    r.message = message;
    r.sourceId = sourceId;
    r.targetId = targetId;
    r.effect = effect;
    return r;
}

static QStringList parseArgs(const QString& argText) {
    QStringList args;
    if (argText.trimmed().isEmpty()) return args;
    args = argText.split(',', Qt::SkipEmptyParts);
    for (QString& a : args) a = a.trimmed();
    return args;
}

CommandParser::CommandParser(GameLevel* level, QObject* parent)
    : QObject(parent), m_level(level) {}

CommandResult CommandParser::execute(const QString& command) {
    if (!m_level) {
        return note("内部错误：CommandParser 没有绑定 GameLevel。");
    }

    const QString text = command.trimmed();
    const QString compact = compactCommand(text);

    if (compact.isEmpty()) {
        return note("请输入一条指令。");
    }

    if (m_level->isWin()) {
        return note("本关已经胜利。");
    }

    if (m_level->isLose()) {
        return note("本关已经失败。");
    }

    const int level = m_level->currentLevel();

    // 非 member-call 的少量语法。
    if (compact == "player+=ruby;" || compact == "player+=ruby") {
        Creature* player = obj(m_level, "player");
        Creature* ruby = obj(m_level, "ruby");
        if (!player || !ruby) return note("找不到 player 或 ruby。", "player", "ruby");
        if (!m_level->flag("level10_mimic_open")) {
            return note("ruby 当前不可访问。", "player", "ruby");
        }
        if (m_level->flag("level10_boosted")) {
            return note("player 已经被强化过。", "player");
        }
        player->addAtk(7);
        ruby->setHp(0);
        m_level->setFlag("level10_boosted");
        return ok("Player::operator+= 生效，player 的 ATK 提升。", "player", "ruby", "buff");
    }

    {
        QRegularExpression swapRe(R"(^swapHp\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)\s*;?$)");
        QRegularExpressionMatch m = swapRe.match(text);
        if (m.hasMatch()) {
            Creature* a = obj(m_level, m.captured(1));
            Creature* b = obj(m_level, m.captured(2));
            if (!a || !b) return note("swapHp 的参数对象不存在。", m.captured(1), m.captured(2));
            int ahp = a->hp();
            a->setHp(b->hp());
            b->setHp(ahp);
            return ok("swapHp(T&, T&) 完成 HP 交换。\n" + joinStatus(a, b), a->id(), b->id(), "buff");
        }
    }

    QRegularExpression memberRe(R"(^([A-Za-z_][A-Za-z0-9_]*)\s*\.\s*([A-Za-z_][A-Za-z0-9_]*)\s*\(([^)]*)\)\s*;?$)");
    QRegularExpressionMatch match = memberRe.match(text);
    if (!match.hasMatch()) {
        return note("无法解析这条指令。请使用 object.method(args); 的形式。");
    }

    const QString sourceId = match.captured(1).trimmed();
    const QString method = match.captured(2).trimmed();
    const QStringList args = parseArgs(match.captured(3));

    Creature* source = obj(m_level, sourceId);
    if (!source) return note(QString("找不到对象：%1。").arg(sourceId), sourceId);
    if (!source->isAlive()) return note(QString("%1 已经失效，不能继续调用成员函数。").arg(sourceId), sourceId);

    const bool isEnemyControl = source->isEnemy() && !m_level->allowEnemyControl();
    if (isEnemyControl) {
        return note(QString("当前关卡不能直接控制 %1。").arg(sourceId), sourceId);
    }

    if (method == "attack") {
        if (args.size() != 1) return note("attack 需要 1 个目标参数。", sourceId);
        Creature* target = obj(m_level, args[0]);
        if (!target) return note(QString("找不到目标：%1。").arg(args[0]), sourceId, args[0]);
        if (!target->isAlive()) return note(QString("%1 已经失效。").arg(target->id()), sourceId, target->id());

        if (level == 4 && sourceId == "mimic") {
            return note("Mimic 的 attack 不是外部可访问接口。", sourceId, target->id());
        }

        if (level == 6 && sourceId == "player" && target->id() == "boss") {
            return ok("virtual onHit() 分发到 ShieldedBoss：护盾吸收了普通攻击。", sourceId, target->id(), "attack");
        }

        if (level == 6 && sourceId == "player" && target->id() == "stone") {
            Creature* player = obj(m_level, "player");
            if (player) player->takeDamage(3);
            return ok("StoneEnemy::onHit() 触发反伤。\n" + hpText(player), sourceId, target->id(), "attack");
        }

        if (level == 6 && sourceId == "player" && target->id() == "bomb") {
            Creature* boss = obj(m_level, "boss");
            if (target) target->setHp(0);
            if (boss) boss->takeDamage(999);
            return ok("VolatileBomb::onHit() 触发爆炸，boss 被波及。\n" + joinStatus(target, boss), sourceId, "boss", "attack");
        }

        const int damage = source->atk();

        if (level == 5 && target->className() == "ArmorBoss" && damage < 6) {
            return ok(QString("ArmorBoss::onHit(%1)：伤害没有穿透护甲。\n%2")
                .arg(damage)
                .arg(hpText(target)), sourceId, target->id(), "attack");
        }

        target->takeDamage(damage);
        return ok(QString("%1.attack(%2); 造成 %3 点伤害。\n%4")
            .arg(sourceId, target->id())
            .arg(damage)
            .arg(hpText(target)), sourceId, target->id(), "attack");
    }

    if (method == "powerAttack") {
        if (args.size() != 1) return note("powerAttack 需要 1 个目标参数。", sourceId);
        Creature* target = obj(m_level, args[0]);
        if (!target) return note(QString("找不到目标：%1。").arg(args[0]), sourceId, args[0]);
        if (!target->isAlive()) return note(QString("%1 已经失效。").arg(target->id()), sourceId, target->id());

        const int damage = source->atk() * 2;
        target->takeDamage(damage);
        return ok(QString("%1.powerAttack(%2); 造成 %3 点伤害。\n%4")
            .arg(sourceId, target->id())
            .arg(damage)
            .arg(hpText(target)), sourceId, target->id(), "attack");
    }

    if (method == "heal") {
        source->heal(4);
        return ok(QString("%1.heal(); 回复生命。\n%2")
            .arg(sourceId)
            .arg(hpText(source)), sourceId, sourceId, "heal");
    }

    if (method == "plantBomb") {
        if (args.size() != 1) return note("plantBomb 需要 1 个目标参数。", sourceId);
        Creature* target = obj(m_level, args[0]);
        if (!target) return note("plantBomb 的目标不存在。", sourceId, args.value(0));
        target->takeDamage(999);
        return ok("ScopedBomb 离开作用域，析构函数造成伤害。\n" + hpText(target), sourceId, target->id(), "attack");
    }

    if (method == "cast") {
        if (args.size() != 1) return note("cast 需要 1 个目标参数。", sourceId);
        Creature* target = obj(m_level, args[0]);
        if (!target) return note("cast 的目标不存在。", sourceId, args.value(0));

        if (sourceId == "slot1") {
            target->takeDamage(6);
            return ok("FireSpell::cast() 造成 6 点伤害。\n" + hpText(target), sourceId, target->id(), "attack");
        }

        if (sourceId == "slot2") {
            target->heal(4);
            return ok("HealSpell::cast() 回复生命。\n" + hpText(target), sourceId, target->id(), "heal");
        }

        if (sourceId == "slot3") {
            const int damage = target->atk();
            target->takeDamage(damage);
            return ok(QString("MirrorSpell::cast() 造成 target.atk() = %1 点伤害。\n%2")
                .arg(damage)
                .arg(hpText(target)), sourceId, target->id(), "attack");
        }

        return note("这个对象没有可用的 cast 实现。", sourceId);
    }

    if (method == "take") {
        if (sourceId == "box3") {
            m_level->setFlag("level9_has_key");
            source->setHp(0);
            return ok("Box<Key>::take() 返回 Key。", sourceId, sourceId, "open");
        }
        source->setHp(0);
        return ok("Box<T>::take() 执行完成。", sourceId, sourceId, "open");
    }

    if (method == "open") {
        if (sourceId == "door") {
            if (!m_level->flag("level9_has_key")) {
                return note("TemplateDoor::open(Key) 缺少 Key 参数。", sourceId);
            }
            source->setHp(0);
            return ok("TemplateDoor::open(Key) 执行，门已打开。", sourceId, sourceId, "open");
        }

        if (sourceId == "mimic") {
            m_level->setFlag("level10_mimic_open");
            source->setHp(0);
            return ok("RubyMimic::open() 执行，ruby 现在可访问。", sourceId, "ruby", "open");
        }

        return note("这个对象不能 open。", sourceId);
    }

    return note(QString("%1 没有可调用的 %2()。").arg(sourceId, method), sourceId);
}
