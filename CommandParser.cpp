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

    // ---- 静态成员函数调用：ClassName::method();（第五关 Berserker::calm()） ----
    {
        QRegularExpression staticRe(R"(^([A-Za-z_][A-Za-z0-9_]*)::([A-Za-z_][A-Za-z0-9_]*)\s*\(\s*\)\s*;?$)");
        QRegularExpressionMatch m = staticRe.match(text);
        if (m.hasMatch()) {
            const QString cls = m.captured(1);
            const QString fn = m.captured(2);
            if (level == 5 && cls == "Berserker" && fn == "calm") {
                m_level->setCounter("level5_rage", 0);
                for (Creature* c : m_level->creatures()) {
                    if (!c || c->className() != "Berserker" || !c->isAlive()) continue;
                    c->setAtk(1);   // 恢复为原始基础攻击力
                    c->setIntent(Creature::IntentAttack, 1);
                }
                return ok("Berserker::calm()（静态成员函数）把共享的 sharedRage 清零，并将所有 Berserker 的攻击力恢复为原值。",
                          QString(), QString(), "buff");
            }
            return note(QString("无法解析静态调用 %1::%2()。").arg(cls, fn));
        }
    }

    // ---- 作用域限定的成员调用：obj.Base::method(args);（第十关 paladin.Healer::expose） ----
    {
        QRegularExpression scopeRe(R"(^([A-Za-z_][A-Za-z0-9_]*)\s*\.\s*([A-Za-z_][A-Za-z0-9_]*)::([A-Za-z_][A-Za-z0-9_]*)\s*\(([^)]*)\)\s*;?$)");
        QRegularExpressionMatch m = scopeRe.match(text);
        if (m.hasMatch()) {
            const QString objId = m.captured(1);
            const QString base = m.captured(2);
            const QString fn = m.captured(3);
            if (level == 10 && objId == "paladin" && fn == "expose" && (base == "Healer" || base == "Warrior")) {
                Creature* paladin = obj(m_level, "paladin");
                Creature* player = obj(m_level, "player");
                if (!paladin || !paladin->isAlive()) return note("paladin 已经失效。", "paladin");
                const int dmg = player ? player->atk() : 3;
                const QString key = (base == "Healer") ? QStringLiteral("level10_healer") : QStringLiteral("level10_warrior");
                int pool = m_level->counter(key, 6) - dmg;
                if (pool < 0) pool = 0;
                m_level->setCounter(key, pool);
                const int total = m_level->counter("level10_healer", 6) + m_level->counter("level10_warrior", 6);
                paladin->setHp(total);
                QString msg = QString("paladin.%1::expose(player)：作用域限定命中 %1 子对象，造成 %2 点。\n%1::hp = %3")
                    .arg(base).arg(dmg).arg(pool);
                if (base == "Healer" && pool == 0) {
                    msg += "\nHealer 子对象被清零，治疗停止。";
                }
                if (total == 0) {
                    msg += "\n两个 Unit 子对象都被清零，Paladin 倒下！";
                }
                return ok(msg, "player", "paladin", "attack");
            }
            return note(QString("无法解析作用域调用 %1.%2::%3()。").arg(objId, base, fn));
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

        if (level == 2 && sourceId == "darkknight") {
            return note("darkknight 的 attack() 不是 public，不能被玩家直接操控。", sourceId, target->id());
        }

        // ---- 第六关：virtual onHit 分发 ----
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

        // ---- 第五关：共享狂暴值（静态成员，越打越疼） ----
        if (level == 5 && target->className() == "Berserker") {
            // 先按玩家攻击力造成伤害
            const int dmg = source->atk();
            target->takeDamage(dmg);
            // 玩家每攻击一次：sharedRage++，然后所有存活 Berserker 的 atk += sharedRage
            int rage = m_level->counter("level5_rage", 0) + 1;
            m_level->setCounter("level5_rage", rage);
            for (Creature* c : m_level->creatures()) {
                if (!c || c->className() != "Berserker" || !c->isAlive()) continue;
                c->setAtk(c->atk() + rage);
                c->setIntent(Creature::IntentAttack, c->atk());
            }
            QString msg = QString("%1.attack(%2) 造成 %3 点伤害。\n所有 Berserker 调用 enrage()：sharedRage → %4，atk += %4（全类共享、越打越疼）。\n%5")
                .arg(sourceId, target->id()).arg(dmg).arg(rage).arg(hpText(target));
            return ok(msg, sourceId, target->id(), "attack");
        }

        // ---- 第四关：对象切片 ----
        if (level == 4 && target->id() == "throne") {
            return note("throne 是用来切片 king 的道具，不要攻击它。", sourceId, target->id());
        }
        if (level == 4 && target->className() == "ImmortalKing") {
            if (!m_level->flag("level4_sliced")) {
                return ok("ImmortalKing::onHit override：王冠之力吸收了全部伤害。\n试着换一种传参方式，把派生部分切掉。",
                          sourceId, target->id(), "attack");
            }
            target->takeDamage(source->atk());
            return ok(QString("king 已被切片成普通 Creature，%1 造成 %2 点伤害。\n%3")
                .arg(sourceId).arg(source->atk()).arg(hpText(target)), sourceId, target->id(), "attack");
        }

        // ---- 第七关：dynamic_cast 向下转型 ----
        if (level == 7 && target->id() == "appraiser") {
            return note("appraiser 是帮你做 dynamic_cast 的鉴定师，不要攻击它。", sourceId, target->id());
        }
        if (level == 7 && target->id() == "decoy") {
            return note("decoy 只是个诱饵箱子（真实类型 DecoyChest），不是要击杀的目标。\n它用来演示 dynamic_cast 在类型不符时返回 nullptr。", sourceId, target->id());
        }
        if (level == 7 && target->className() == "Mimic") {
            if (!m_level->flag("level7_revealed")) {
                return ok("Mimic 仍处于伪装：重写的 onHit 吸收了全部伤害。\n它的弱点 revealWeakness() 只存在于 Mimic 派生类——你得先用 dynamic_cast 拿到它的真身接口。",
                          sourceId, target->id(), "attack");
            }
            target->takeDamage(source->atk());
            return ok(QString("伪装已被剥下，%1 造成 %2 点伤害。\n%3")
                .arg(sourceId).arg(source->atk()).arg(hpText(target)), sourceId, target->id(), "attack");
        }

        // ---- 第八关：道具与金库本体 ----
        if (level == 8 && (target->id() == "saboteur" || target->id() == "rogue")) {
            return note(QString("%1 是场上的人物道具，不要攻击它。").arg(target->id()), sourceId, target->id());
        }
        if (level == 8 && target->className() == "Vault") {
            return ok("Vault 外壳无懈可击，常规攻击无效。\n它的弱点 selfDestruct() 是 private——只有它的 friend 才能触发。",
                      sourceId, target->id(), "attack");
        }

        // ---- 第九关：命匣与引用计数 ----
        if (level == 9 && target->className() == "SoulGem") {
            return note("SoulGem 的寿命由 shared_ptr 的引用计数决定，不能直接攻击。\n让 use_count 归零（消灭所有持有者）才行。",
                        sourceId, target->id());
        }
        if (level == 9 && target->className() == "Lich") {
            const int rc = m_level->counter("level9_refcount", 3);
            if (rc > 0) {
                return ok(QString("Lich 通过 weak_ptr 观察 SoulGem：use_count = %1 > 0，命匣未碎，Lich 免疫。").arg(rc),
                          sourceId, target->id(), "attack");
            }
            target->takeDamage(source->atk());
            return ok(QString("命匣已析构，Lich 失去庇护，%1 造成 %2 点伤害。\n%3")
                .arg(sourceId).arg(source->atk()).arg(hpText(target)), sourceId, target->id(), "attack");
        }
        if (level == 9 && target->className() == "Cultist") {
            target->takeDamage(source->atk());
            QString msg = QString("%1.attack(%2) 造成 %3 点伤害。\n%4")
                .arg(sourceId, target->id()).arg(source->atk()).arg(hpText(target));
            if (!target->isAlive()) {
                int rc = m_level->counter("level9_refcount", 3) - 1;
                if (rc < 0) rc = 0;
                m_level->setCounter("level9_refcount", rc);
                Creature* gem = obj(m_level, "gem");
                if (gem) gem->setHp(rc);
                msg += QString("\n%1 释放了它的 shared_ptr，SoulGem use_count → %2。").arg(target->id()).arg(rc);
                if (rc == 0) {
                    msg += "\nuse_count 归零，~SoulGem() 析构，Lich 的 weak_ptr 失效——Lich 现在可被攻击！";
                }
            }
            return ok(msg, sourceId, target->id(), "attack");
        }

        // ---- 第十关：直接攻击 paladin 有二义性 ----
        if (level == 10 && target->className() == "Paladin") {
            return note("直接攻击 paladin 会触及二义的 hp（Healer 与 Warrior 各带来一个 Unit::hp 子对象）。\n请用 paladin.Healer::expose(player) 或 paladin.Warrior::expose(player)。",
                        sourceId, target->id());
        }

        // ---- 默认普通攻击 ----
        const int damage = source->atk();
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

    if (method == "summonBomb") {
        if (level != 3 || sourceId != "player") return note("当前关卡没有可调用的 summonBomb()。", sourceId);
        if (!args.isEmpty()) return note("summonBomb 不需要参数。", sourceId);
        QString bombId;
        int bombIndex = -1;
        for (int i = 1; i <= 4; ++i) {
            const QString candidate = QString("bomb%1").arg(i);
            if (!obj(m_level, candidate)) { bombId = candidate; bombIndex = i; break; }
        }
        if (bombIndex < 0) return note("最多同时存在 4 个 bomb。", sourceId);
        Creature* player = obj(m_level, "player");
        if (!player) return note("找不到 player。", sourceId);
        Creature* bomb = new Creature(bombId, QString("Bomb %1").arg(bombIndex), "Player::Bomb", 1, 0, false, m_level);
        bomb->addClassDeclaration("Bomb(Player& owner);");
        bomb->addClassDeclaration("void countdown();");
        bomb->addClassDeclaration("~Bomb();");
        bomb->setDescription(QStringLiteral("由 player 召唤的全局 Bomb：存在 3 个玩家行动回合后自动析构，对所有敌方造成 10 点伤害；存在期间令 player.DEF + 2。"));
        bomb->setExtraCodeHtml(
            "<div style=\"margin-top:12px; color:#FFD36E; font-weight:900; font-size:18px;\">Bomb 实现</div>"
            "<pre style=\"font-size:17px; line-height:1.35; white-space:pre-wrap;\">"
            "Player::Bomb::Bomb(Player&amp; owner)\n"
            "    : Creature(&quot;bomb&quot;, &quot;Bomb&quot;, &quot;Player::Bomb&quot;, 1, 0, false),\n"
            "      owner(owner), ttl(3) {\n"
            "    owner.def += 2;\n"
            "}\n\n"
            "void Player::Bomb::countdown() {\n"
            "    ttl--;\n"
            "    if (ttl &lt;= 0) {\n"
            "        delete this;   // 触发 ~Bomb()\n"
            "    }\n"
            "}\n\n"
            "Player::Bomb::~Bomb() {\n"
            "    for (Creature&amp; enemy : world.enemies()) {\n"
            "        enemy.takeDamage(10);\n"
            "    }\n"
            "    owner.def -= 2;\n"
            "}\n"
            "</pre>"
        );
        bomb->setIntent(Creature::IntentUnknown, 4, QStringLiteral("倒计时"));
        bomb->setIntentFunctionName(QStringLiteral("countdown"));
        m_level->addCreature(bombId, bomb);
        player->addDef(2);
        m_level->setCounter(QString("level3_bomb_ttl_%1").arg(bombId), 4);
        return ok(QString("player.summonBomb(); 召唤 %1，player DEF +2。").arg(bombId), sourceId, bombId, "buff");
    }

    if (method == "defend") {
        if (level != 3 || sourceId != "player") return note("当前关卡没有可调用的 defend()。", sourceId);
        if (!args.isEmpty()) return note("defend 不需要参数。", sourceId);
        source->setShield(source->def());
        return ok(QString("player.defend(); 获得 %1 点护盾。").arg(source->def()), sourceId, sourceId, "buff");
    }

    if (method == "heal") {
        if (level == 1) return note("当前关卡没有可调用的 heal()。", sourceId);
        source->heal(4);
        return ok(QString("%1.heal(); 回复生命。\n%2")
            .arg(sourceId)
            .arg(hpText(source)), sourceId, sourceId, "heal");
    }

    // ---- 第四关：穿刺共享的心脏 ----
    // ---- 第四关：王座的两个接口 ----
    if (method == "seat") {
        if (level != 4 || sourceId != "throne") return note("当前关卡没有可调用的 seat()。", sourceId);
        if (args.size() != 1) return note("seat 需要 1 个参数。", sourceId);
        if (args[0] != "king") return note("本关的 seat() 用来演示 throne.seat(king)。", sourceId, args.value(0));
        Creature* king = obj(m_level, "king");
        if (!king) return note("找不到 king。", sourceId);
        if (m_level->flag("level4_sliced")) return note("king 已经被 place(c) 替换成普通 Creature 了。", sourceId, "king");

        Creature* sliced = new Creature(
            QStringLiteral("king"),
            QStringLiteral("Sliced King"),
            QStringLiteral("Creature"),
            king->maxHp(),
            king->atk(),
            true,
            m_level
        );
        sliced->setHp(king->hp());
        sliced->setIntent(Creature::IntentAttack, king->atk());
        sliced->setDescription(QStringLiteral("这是 throne.seat(king) 中的形参 Creature c 被 place(c) 放回场上后的对象。它取代了原来的 ImmortalKing；派生部分和 onHit override 已不存在。"));
        sliced->setExtraCodeHtml(
            "<div style=\"margin-top:12px; color:#FFD36E; font-weight:900; font-size:18px;\">place(c) 替换结果</div>"
            "<pre style=\"font-size:17px; line-height:1.35; white-space:pre-wrap;\">"
            "// seat(Creature c) 里的 c 已经是基类 Creature 副本。\n"
            "// place(c) 把这份 c 放回 id=\"king\" 的位置：\n"
            "world.replace(\"king\", c);\n\n"
            "// 因此当前 king 不再是 ImmortalKing，\n"
            "// 没有 ImmortalKing::onHit override，普通攻击会正常扣血。"
            "</pre>"
        );
        m_level->addCreature(QStringLiteral("king"), sliced);
        m_level->setFlag("level4_sliced");
        return ok("throne.seat(Creature c)：按值传参先把 ImmortalKing 拷贝成普通 Creature c；随后 place(c) 把这份 c 放回 king 的位置，取代原来的 ImmortalKing。现在场上的 king 是普通 Creature，可以正常受伤。",
                  sourceId, "king", "buff");
    }
    if (method == "crown") {
        if (level != 4 || sourceId != "throne") return note("当前关卡没有可调用的 crown()。", sourceId);
        return note("throne.crown(Creature& c)：c 是原 king 的引用；place(c) 放回的仍是同一个 ImmortalKing，因此派生部分和 onHit override 都保留，不会切片。",
                    sourceId, "king");
    }

    // ---- 第七关：dynamic_cast 向下转型 ----
    if (method == "reveal") {
        if (level != 7 || sourceId != "appraiser") return note("当前关卡没有可调用的 reveal()。", sourceId);
        if (args.size() != 1) return note("reveal 需要 1 个目标参数。", sourceId);
        Creature* target = obj(m_level, args[0]);
        if (!target) return note(QString("找不到目标：%1。").arg(args[0]), sourceId, args.value(0));
        if (target->className() == "Mimic") {
            if (m_level->flag("level7_revealed")) {
                return note("mimic 的伪装已经被剥下了，不用再 reveal。", sourceId, target->id());
            }
            m_level->setFlag("level7_revealed");
            return ok("dynamic_cast<Mimic*>(&c) 命中：c 的真实类型确实是 Mimic，返回有效指针。\nappraiser 调用它派生类独有的 revealWeakness()，拟态魔的伪装被剥下——现在可以正常攻击 mimic 了。",
                      sourceId, target->id(), "buff");
        }
        return note(QString("dynamic_cast<Mimic*>(&%1) 返回 nullptr：%1 的真实类型不是 Mimic，向下转型安全失败"
                            "（不会像 static_cast 那样产生未定义行为）。").arg(target->id()), sourceId, target->id());
    }
    if (method == "revealWeakness") {
        if (level == 7) {
            return note("revealWeakness() 是 Mimic 派生类特有的成员。\n场上你拿到的是 Creature& 基类视图，编译期看不到这个方法，不能直接调用。\n需要先用 dynamic_cast 向下转型成 Mimic——appraiser.reveal(mimic) 就是这么做的。",
                        sourceId);
        }
        return note(QString("%1 没有可直接调用的 revealWeakness()。").arg(sourceId), sourceId);
    }

    // ---- 第八关：内鬼 / 盗贼 / 私有自毁 ----
    if (method == "betray") {
        if (level != 8 || sourceId != "saboteur") return note("当前关卡没有可调用的 betray()。", sourceId);
        Creature* vault = obj(m_level, "vault");
        if (!vault) return note("找不到 vault。", sourceId);
        if (!vault->isAlive()) return note("vault 已经失效。", sourceId, "vault");
        vault->setHp(0);
        return ok("Saboteur 是 Vault 的 friend，betray() 内部合法调用了 vault.selfDestruct()，金库自毁！", sourceId, "vault", "attack");
    }
    if (method == "pick") {
        if (level != 8 || sourceId != "rogue") return note("当前关卡没有可调用的 pick()。", sourceId);
        return note("Rogue 没有出现在 Vault 的 friend 声明里，无法访问它的 private 成员 selfDestruct()。", sourceId, "vault");
    }
    if (method == "selfDestruct") {
        if (level == 8 && sourceId == "vault") {
            return note("selfDestruct() 是 Vault 的 private 成员，外部无法直接调用。\n看看 Vault 的类体里把谁声明成了 friend。", sourceId);
        }
        return note(QString("%1 没有可调用的 selfDestruct()。").arg(sourceId), sourceId);
    }

    // ---- 第十关：未限定的 expose 有二义性 ----
    if (method == "expose") {
        if (level == 10 && sourceId == "paladin") {
            return note("paladin.expose(...) 有二义性：Healer 与 Warrior 各带来一个 expose（以及各自的 Unit::hp）。\n请用 paladin.Healer::expose(player) 或 paladin.Warrior::expose(player) 作用域限定。",
                        sourceId);
        }
        return note(QString("%1 没有可直接调用的 expose()。").arg(sourceId), sourceId);
    }

    return note(QString("%1 没有可调用的 %2()。").arg(sourceId, method), sourceId);
}
