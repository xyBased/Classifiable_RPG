#include "CommandParser.h"

#include <QRegularExpression>
#include <QStringList>

namespace {

QString compactCommand(QString command) {
    command.remove(' ');
    command.remove('\t');
    command.remove('\n');
    command.remove('\r');
    return command;
}

CommandResult makeResult(bool success, bool consumeStep, const QString& message, const QString& sourceId = QString()) {
    CommandResult r;
    r.success = success;
    r.consumeStep = consumeStep;
    r.message = message;
    r.sourceId = sourceId;
    return r;
}

CommandResult ok(const QString& message, const QString& sourceId = QString(), bool consumeStep = true) {
    return makeResult(true, consumeStep, message, sourceId);
}

CommandResult note(const QString& message, const QString& sourceId = QString()) {
    return makeResult(false, false, message, sourceId);
}

Creature* obj(GameLevel* level, const QString& id) {
    return level ? level->creature(id) : nullptr;
}

QString hpText(Creature* c) {
    if (!c) return QString();
    return QString("%1 HP：%2/%3").arg(c->id()).arg(c->hp()).arg(c->maxHp());
}

CommandResult requireObject(GameLevel* level, const QString& id) {
    if (!obj(level, id)) {
        return note(QString("找不到对象：%1。请检查对象名是否与场景中的 id 一致。").arg(id));
    }
    return CommandResult{};
}

QString joinStatus(Creature* a, Creature* b = nullptr) {
    QString s;
    if (a) s += hpText(a);
    if (b) {
        if (!s.isEmpty()) s += "\n";
        s += hpText(b);
    }
    return s;
}

bool isExact(const QString& compact, const QString& expected) {
    return compact == compactCommand(expected);
}

} // namespace

CommandParser::CommandParser(GameLevel* level, QObject* parent)
    : QObject(parent), m_level(level) {}

CommandResult CommandParser::execute(const QString& command) {
    if (!m_level) {
        return note("内部错误：CommandParser 没有绑定 GameLevel。");
    }

    const QString text = command.trimmed();
    const QString compact = compactCommand(text);

    if (compact.isEmpty()) {
        return note("请输入一条指令，例如 player.attack(enemy);。");
    }

    if (m_level->isWin()) {
        return note("本关已经胜利，可以进入下一关。");
    }
    if (m_level->isLose()) {
        return note("本关已经失败，请重试。");
    }

    const int level = m_level->currentLevel();

    if (level == 3) {
        Creature* enemy = obj(m_level, "enemy");
        if (isExact(compact, "player.plantBomb(enemy);")) {
            if (!enemy) return requireObject(m_level, "enemy");
            enemy->takeDamage(999);
            return ok(
                QStringLiteral("ScopedBomb(enemy) 构造：炸弹绑定到 enemy。\n"
                               "plantBomb() 即将结束，局部对象 bomb 离开作用域。\n"
                               "~ScopedBomb() 析构：enemy 受到 999 点伤害。\n") + hpText(enemy),
                "player"
                );
        }
        if (isExact(compact, "player.attack(enemy);") || isExact(compact, "player.powerAttack(enemy);")) {
            return ok(
                "IronWall::takeDamage(damage) 被调用。\n"
                "由于 damage < 999，伤害被直接 return 掉。\n"
                "提示：真正的伤害写在 ~ScopedBomb() 里。",
                "player"
                );
        }
    }

    if (level == 4) {
        Creature* goblin = obj(m_level, "goblin");
        Creature* boss = obj(m_level, "boss");
        if (isExact(compact, "goblin.attack(boss);")) {
            if (!goblin || !boss) return note("找不到 goblin 或 boss。");
            boss->takeDamage(goblin->atk());
            return ok(
                QStringLiteral("Goblin 没有自己声明 attack()，但它 public 继承 Enemy。\n"
                               "所以 Enemy::attack(Creature& target) 仍是 Goblin 的 public 接口。\n"
                               "goblin 攻击 boss，造成 12 点伤害。\n") + hpText(boss),
                "goblin"
                );
        }
        if (isExact(compact, "mimic.attack(boss);")) {
            return note(
                "编译器风格错误：Enemy 是 Mimic 的 private 基类。\n"
                "Enemy::attack() 原本是 public，但 private 继承后不能被外部通过 mimic 访问。",
                "mimic"
                );
        }
        if (isExact(compact, "skeleton.attack(boss);")) {
            return note(
                "编译器风格错误：Enemy 是 Skeleton 的 protected 基类。\n"
                "外部代码不能通过 skeleton 访问继承来的 Enemy::attack()。",
                "skeleton"
                );
        }
    }

    if (level == 5) {
        Creature* player = obj(m_level, "player");
        Creature* enemy = obj(m_level, "enemy");
        Creature* ruby = obj(m_level, "ruby");

        if (isExact(compact, "player+ruby;")) {
            return note(
                "语句合法，但没有改变原 player。\n"
                "Player::operator+(const Gem&) 返回的是一个强化后的临时 Player；\n"
                "你没有接收返回值，所以临时对象在语句结束后消失。"
                );
        }
        if (isExact(compact, "ruby+player;")) {
            return note(
                "这里没有定义 Gem::operator+(Player)。\n"
                "运算符重载也要看参数顺序：player + ruby 和 ruby + player 不是一回事。"
                );
        }
        if (isExact(compact, "player+=ruby;")) {
            if (!player || !ruby) return note("找不到 player 或 ruby。");
            if (m_level->flag("level5_player_boosted")) {
                return note("ruby 已经被 operator+= 消耗过了，不能重复强化。", "player");
            }
            player->addAtk(7);
            ruby->setHp(0);
            m_level->setFlag("level5_player_boosted");
            return ok(
                "Player::operator+=(const Gem&) 被调用。\n"
                "this->atk += gem.power(); 修改了 player 本体。\n"
                "player ATK 现在是 10，可以突破 ArmorBoss 的护甲。",
                "player"
                );
        }
        if (isExact(compact, "player.attack(enemy);") || isExact(compact, "player.powerAttack(enemy);")) {
            if (!player || !enemy) return note("找不到 player 或 enemy。");
            int damage = player->atk();
            if (isExact(compact, "player.powerAttack(enemy);")) damage *= 2;
            if (damage < 10) {
                return ok(
                    "ArmorBoss::takeDamage(damage) 被调用。\n"
                    "damage < 10，护甲忽略了这次攻击。\n"
                    "提示：比较 operator+ 和 operator+=。",
                    "player"
                    );
            }
            enemy->takeDamage(damage);
            return ok(
                QString("damage = %1，已经达到护甲阈值。\nArmorBoss 受到伤害。\n%2")
                    .arg(damage)
                    .arg(hpText(enemy)),
                "player"
                );
        }
    }

    if (level == 6) {
        Creature* player = obj(m_level, "player");
        Creature* boss = obj(m_level, "boss");
        Creature* stone = obj(m_level, "stone");
        Creature* bomb = obj(m_level, "bomb");

        if (isExact(compact, "player.attack(boss);")) {
            return ok(
                "Player::attack(Creature& target) 被调用。\n"
                "target 的静态类型是 Creature&，实际类型是 ShieldedBoss。\n"
                "因为 onHit 是 virtual，执行 ShieldedBoss::onHit()：护盾吸收攻击。",
                "player"
                );
        }
        if (isExact(compact, "player.attack(stone);")) {
            if (!player) return note("找不到 player。");
            player->takeDamage(3);
            return ok(
                QStringLiteral("target 的实际类型是 StoneEnemy。\n"
                               "virtual dispatch 执行 StoneEnemy::onHit()。\n"
                               "StoneEnemy 反伤 player 3 点。\n") + hpText(player),
                "player"
                );
        }
        if (isExact(compact, "player.attack(bomb);")) {
            if (!boss || !bomb) return note("找不到 boss 或 bomb。");
            bomb->setHp(0);
            boss->takeDamage(20);
            return ok(
                QStringLiteral("target 的实际类型是 BombEnemy。\n"
                               "virtual dispatch 执行 BombEnemy::onHit()。\n"
                               "bomb 爆炸，boss 受到 20 点伤害。\n") + joinStatus(bomb, boss),
                "player"
                );
        }
    }

    if (level == 7) {
        Creature* boss = obj(m_level, "boss");
        if (!boss) return note("找不到 boss。");

        auto healIfNeeded = [&]() -> QString {
            if (!m_level->flag("level7_silenced") && boss->isAlive()) {
                boss->heal(8);
                return "\nVampireBoss::afterTurn()：boss 没有被 silence，回复 8 点。";
            }
            if (m_level->flag("level7_silenced")) {
                return "\nVampireBoss::afterTurn()：回血已被 SilenceSpell 封印。";
            }
            return QString();
        };

        if (isExact(compact, "slot2.cast(boss);")) {
            m_level->setFlag("level7_silenced");
            return ok(
                "slot2 的静态类型是 Spell*，实际对象是 SilenceSpell。\n"
                "调用 SilenceSpell::cast()，boss 的 afterTurn 回血被封印。",
                "slot2"
                );
        }
        if (isExact(compact, "slot1.cast(boss);")) {
            boss->takeDamage(6);
            QString msg =
                "slot1 的静态类型是 Spell*，实际对象是 FireSpell。\n"
                "调用 FireSpell::cast()，boss 受到 6 点伤害。";
            msg += healIfNeeded();
            msg += QStringLiteral("\n") + hpText(boss);
            return ok(msg, "slot1");
        }
        if (isExact(compact, "slot3.cast(boss);")) {
            const int damage = boss->atk();
            boss->takeDamage(damage);
            QString msg = QString(
                              "slot3 的静态类型是 Spell*，实际对象是 MirrorSpell。\n"
                              "调用 MirrorSpell::cast()，伤害等于 target.atk() = %1。"
                              ).arg(damage);
            msg += healIfNeeded();
            msg += QStringLiteral("\n") + hpText(boss);
            return ok(msg, "slot3");
        }
    }

    if (level == 8) {
        Creature* enemy = obj(m_level, "enemy");
        Creature* poisonBomb = obj(m_level, "poisonBomb");
        if (!enemy) return note("找不到 enemy。");

        if (isExact(compact, "use(healPotion,enemy);")) {
            enemy->heal(5);
            return ok(
                QStringLiteral("template <typename Item> void use(Item& item, Creature& target)\n"
                               "第一个实参是 HealPotion，所以 Item 被推导为 HealPotion。\n"
                               "调用 HealPotion::apply(enemy)：enemy 被治疗。\n") + hpText(enemy),
                "healPotion"
                );
        }
        if (isExact(compact, "use(poisonBomb,enemy);")) {
            enemy->takeDamage(20);
            if (poisonBomb) poisonBomb->setHp(0);
            return ok(
                QStringLiteral("template <typename Item> void use(Item& item, Creature& target)\n"
                               "第一个实参是 PoisonBomb，所以 Item 被推导为 PoisonBomb。\n"
                               "调用 PoisonBomb::apply(enemy)：enemy 受到 20 点伤害。\n") + hpText(enemy),
                "poisonBomb"
                );
        }
        if (compact.startsWith("use(")) {
            return note(
                "模板函数 use(Item&, Creature&) 要求第一个参数拥有 apply(Creature&) 方法。\n"
                "请检查右侧道具类的 apply() 实现。"
                );
        }
    }

    if (level == 9) {
        Creature* enemy = obj(m_level, "enemy");
        Creature* door = obj(m_level, "door");
        Creature* box1 = obj(m_level, "box1");
        Creature* box2 = obj(m_level, "box2");
        Creature* box3 = obj(m_level, "box3");

        if (isExact(compact, "box1.take();")) {
            if (box1) box1->setHp(0);
            m_level->setFlag("level9_has_potion");
            return ok(
                "box1 的类型是 Box<HealPotion>，take() 返回 HealPotion。\n"
                "你拿到了药水，但 TemplateDoor::open(Key) 不接受 HealPotion。",
                "box1"
                );
        }
        if (isExact(compact, "box3.take();")) {
            if (box3) box3->setHp(0);
            m_level->setFlag("level9_has_key");
            return ok(
                "box3 的类型是 Box<Key>，take() 返回 Key。\n"
                "现在可以调用 door.open(key);。",
                "box3"
                );
        }
        if (isExact(compact, "door.open(key);")) {
            if (!m_level->flag("level9_has_key")) {
                return note(
                    "TemplateDoor::open(Key) 需要一个 Key。\n"
                    "先观察三个 Box<T> 的 T，找出 Box<Key>。",
                    "door"
                    );
            }
            if (door) door->setHp(0);
            m_level->setFlag("level9_door_open");
            return ok(
                "TemplateDoor::open(Key) 调用成功。\n"
                "门已打开，现在敌人不再被保护。",
                "door"
                );
        }
        if (isExact(compact, "box2.take();")) {
            if (box2) box2->setHp(0);
            m_level->setFlag("level9_has_bomb");
            return ok(
                "box2 的类型是 Box<PoisonBomb>，take() 返回 PoisonBomb。\n"
                "现在可以对 enemy 使用 use(poisonBomb, enemy);。",
                "box2"
                );
        }
        if (isExact(compact, "use(poisonBomb,enemy);")) {
            if (!m_level->flag("level9_door_open")) {
                return note("TemplateDoor 还没打开，PoisonBomb 被门挡住了。", "poisonBomb");
            }
            if (!m_level->flag("level9_has_bomb")) {
                return note("你还没有从 Box<PoisonBomb> 中取出 PoisonBomb。", "poisonBomb");
            }
            if (!enemy) return note("找不到 enemy。");
            enemy->takeDamage(20);
            return ok(
                QStringLiteral("函数模板 use 的 Item 被推导为 PoisonBomb。\n"
                               "PoisonBomb::apply(enemy) 生效。\n") + hpText(enemy),
                "poisonBomb"
                );
        }
    }

    if (level == 10) {
        Creature* player = obj(m_level, "player");
        Creature* boss = obj(m_level, "boss");
        Creature* mimic = obj(m_level, "mimic");
        Creature* ruby = obj(m_level, "ruby");
        Creature* bomb = obj(m_level, "bomb");
        Creature* weakDragon = obj(m_level, "weakDragon");

        if (isExact(compact, "mimic.open();")) {
            if (mimic) mimic->setHp(0);
            m_level->setFlag("level10_mimic_open");
            return ok(
                "Mimic public 继承 Chest。\n"
                "Chest::open() 作为 public 接口暴露给外部，mimic 被打开。\n"
                "里面露出了 ruby。",
                "mimic"
                );
        }
        if (isExact(compact, "player+ruby;")) {
            return note(
                "Player::operator+ 返回强化后的临时对象，但没有改变原 player。\n"
                "终局也不会放过临时对象陷阱。"
                );
        }
        if (isExact(compact, "player+=ruby;")) {
            if (!player || !ruby) return note("找不到 player 或 ruby。");
            if (!m_level->flag("level10_mimic_open")) {
                return note("ruby 还藏在 mimic 里。先利用 public 继承来的 open()。", "player");
            }
            if (m_level->flag("level10_boosted")) {
                return note("player 已经被 ruby 强化过了。", "player");
            }
            player->addAtk(7);
            ruby->setHp(0);
            m_level->setFlag("level10_boosted");
            return ok(
                "Player::operator+= 修改 player 本体。\n"
                "player ATK 现在是 10。",
                "player"
                );
        }
        if (isExact(compact, "slot.cast(boss);")) {
            if (!boss) return note("找不到 boss。");
            m_level->setFlag("level10_silenced");
            return ok(
                "slot 的静态类型是 Spell*，实际对象是 SilenceSpell。\n"
                "virtual dispatch 执行 SilenceSpell::cast()。\n"
                "UndefinedBoss 的类型护盾被暂时封印。",
                "slot"
                );
        }
        if (isExact(compact, "player.attack(boss);")) {
            return ok(
                "UndefinedBoss 的 typeShield 仍在保护本体。\n"
                "直接攻击不是终局答案；你需要先利用 BombEnemy::onHit()。",
                "player"
                );
        }
        if (isExact(compact, "player.attack(bomb);")) {
            if (!boss || !bomb || !player) return note("找不到 boss、bomb 或 player。");
            if (!m_level->flag("level10_silenced")) {
                player->takeDamage(4);
                return ok(
                    QStringLiteral("BombEnemy::onHit() 被触发，但 UndefinedBoss 的护盾没有被封印。\n"
                                   "爆炸被反弹，player 受到 4 点伤害。\n") + hpText(player),
                    "player"
                    );
            }
            bomb->setHp(0);
            boss->takeDamage(25);
            m_level->setFlag("level10_bombed");
            return ok(
                QStringLiteral("target 的实际类型是 BombEnemy。\n"
                               "virtual dispatch 执行 BombEnemy::onHit()。\n"
                               "由于 boss 已被 SilenceSpell 封印，爆炸成功命中。\n") + joinStatus(bomb, boss),
                "player"
                );
        }
        if (isExact(compact, "swapHp(weakDragon,boss);")) {
            if (!boss || !weakDragon) return note("找不到 weakDragon 或 boss。");
            if (!m_level->flag("level10_bombed")) {
                return note(
                    "swapHp(T&, T&) 的时机还不对。\n"
                    "先通过 virtual onHit 炸开 UndefinedBoss 的类型护盾。",
                    "weakDragon"
                    );
            }
            if (m_level->flag("level10_swapped")) {
                return note("HP 已经交换过了，继续执行不会带来新变化。", "weakDragon");
            }
            const int bossHp = boss->hp();
            const int weakHp = weakDragon->hp();
            if (weakDragon->maxHp() < bossHp) weakDragon->setMaxHp(bossHp);
            boss->setHp(weakHp);
            weakDragon->setHp(bossHp);
            m_level->setFlag("level10_swapped");
            return ok(
                QStringLiteral("template <class T> void swapHp(T& a, T& b) 调用成功。\n"
                               "weakDragon 与 boss 在本关被视作同一 DragonPart 系列，T 可以一致。\n"
                               "boss 的 HP 被换成 weakDragon 的 1 点。\n") + joinStatus(weakDragon, boss),
                "weakDragon"
                );
        }
        if (isExact(compact, "player.plantBomb(boss);")) {
            if (!boss) return note("找不到 boss。");
            if (!m_level->flag("level10_swapped")) {
                return ok(
                    "ScopedBomb 在作用域结束时析构并爆炸。\n"
                    "但 UndefinedBoss 还没有被 swapHp 削弱，爆炸被残余护盾抵消。",
                    "player"
                    );
            }
            boss->takeDamage(999);
            return ok(
                QStringLiteral("ScopedBomb(boss) 构造。\n"
                               "player.plantBomb(boss) 结束，局部对象离开作用域。\n"
                               "~ScopedBomb() 析构：boss 受到 999 点伤害。\n"
                               "UndefinedBoss 已被定义。\n") + hpText(boss),
                "player"
                );
        }
    }

    static const QRegularExpression re(
        R"(^([A-Za-z_][A-Za-z0-9_]*)\.([A-Za-z_][A-Za-z0-9_]*)\(([^()]*)\);$)"
        );
    const QRegularExpressionMatch match = re.match(compact);
    if (!match.hasMatch()) {
        return note(
            "语法未识别。当前基础解析器支持：\n"
            "1. object.method(target);\n"
            "2. object.method();\n"
            "部分关卡还支持 player += ruby;、use(item,target);、swapHp(a,b); 等特殊指令。"
            );
    }

    const QString sourceId = match.captured(1);
    const QString method = match.captured(2);
    const QString argText = match.captured(3).trimmed();
    QStringList args;
    if (!argText.isEmpty()) {
        args = argText.split(',', Qt::SkipEmptyParts);
        for (QString& arg : args) arg = arg.trimmed();
    }

    Creature* source = obj(m_level, sourceId);
    if (!source) {
        return note(QString("找不到对象：%1。").arg(sourceId));
    }
    if (!source->isAlive()) {
        return note(QString("%1 已经失效，不能继续调用成员函数。").arg(sourceId), sourceId);
    }

    if (source->isEnemy() && !m_level->allowEnemyControl()) {
        return note(
            QString("不能直接控制 %1。\n提示：只有特定关卡会把某些对象的 public 接口开放给玩家。")
                .arg(sourceId),
            sourceId
            );
    }

    if (method == "attack") {
        if (args.size() != 1) return note("attack 需要且只需要一个目标参数。", sourceId);
        Creature* target = obj(m_level, args[0]);
        if (!target) return note(QString("找不到目标：%1。").arg(args[0]), sourceId);
        if (!target->isAlive()) return note(QString("%1 已经没有生命值，不需要继续攻击。").arg(args[0]), sourceId);
        if (target == source) return note("为了避免误操作，基础 attack 不允许攻击自己。", sourceId);
        target->takeDamage(source->atk());
        return ok(
            QString("%1.attack(%2) 执行成功，造成 %3 点伤害。\n%4")
                .arg(sourceId, args[0])
                .arg(source->atk())
                .arg(hpText(target)),
            sourceId
            );
    }

    if (method == "powerAttack") {
        if (source->id() != "player") return note("powerAttack 是 Player 的接口。", sourceId);
        if (args.size() != 1) return note("powerAttack 需要一个目标参数。", sourceId);
        Creature* target = obj(m_level, args[0]);
        if (!target) return note(QString("找不到目标：%1。").arg(args[0]), sourceId);
        const int damage = source->atk() * 2;
        target->takeDamage(damage);
        return ok(
            QString("player.powerAttack(%1) 执行成功，造成 %2 点伤害。\n%3")
                .arg(args[0])
                .arg(damage)
                .arg(hpText(target)),
            sourceId
            );
    }

    if (method == "heal") {
        if (source->id() != "player") return note("heal 是 Player 的接口。", sourceId);
        if (!args.isEmpty()) return note("heal() 不需要参数。", sourceId);
        source->heal(3);
        return ok(QStringLiteral("player.heal() 执行成功，回复 3 点生命。\n") + hpText(source), sourceId);
    }

    return note(
        QString("对象 %1 没有当前可执行的 public 方法：%2。\n请观察右侧类代码中的可点击成员或本关特殊语法。")
            .arg(sourceId, method),
        sourceId
        );
}
