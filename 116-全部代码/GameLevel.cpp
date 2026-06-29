#include "GameLevel.h"

#include <QtGlobal>

GameLevel::GameLevel(QObject* parent) : QObject(parent) {}

QString GameLevel::levelName(int level) {
    switch (level) {
    case 1: return QStringLiteral("平A穿插普攻");
    case 2: return QStringLiteral("借刀清场");
    case 3: return QStringLiteral("析构爆破");
    case 4: return QStringLiteral("对象切片");
    case 5: return QStringLiteral("静态狂暴");
    case 6: return QStringLiteral("虚函数分发");
    case 7: return QStringLiteral("真身显形");
    case 8: return QStringLiteral("友元内鬼");
    case 9: return QStringLiteral("共享所有权");
    case 10: return QStringLiteral("菱形继承");
    default: return QStringLiteral("未命名关卡");
    }
}
GameLevel::~GameLevel() {
    qDeleteAll(m_creatures);
    m_creatures.clear();
}

QString GameLevel::htmlBlock(const QString& title, const QString& code) const {
    return QString(
        "<div style=\"margin-top:12px; color:#FFD36E; font-weight:900; font-size:18px;\">%1</div>"
        "<pre style=\"font-size:17px; line-height:1.35; white-space:pre-wrap;\">%2</pre>"
    ).arg(title.toHtmlEscaped(), code.toHtmlEscaped());
}

void GameLevel::loadLevel(int n) {
    switch (n) {
    case 1: loadLevel1(); break;
    case 2: loadLevel2(); break;
    case 3: loadLevel3(); break;
    case 4: loadLevel4(); break;
    case 5: loadLevel5(); break;
    case 6: loadLevel6(); break;
    case 7: loadLevel7(); break;
    case 8: loadLevel8(); break;
    case 9: loadLevel9(); break;
    case 10: loadLevel10(); break;
    default: loadLevel1(); break;
    }
}

void GameLevel::resetCreatures() {
    qDeleteAll(m_creatures);
    m_creatures.clear();
    m_flags.clear();
    m_counters.clear();
    m_usedSteps = 0;
    m_allowEnemyControl = false;
    m_intro.clear();
    m_levelName.clear();
}

void GameLevel::registerCreature(const QString& id, Creature* c) {
    m_creatures.insert(id, c);
    connect(c, &Creature::changed, this, &GameLevel::levelChanged);
}

void GameLevel::addCreature(const QString& id, Creature* c) {
    if (!c) return;
    if (m_creatures.contains(id)) {
        Creature* old = m_creatures.take(id);
        delete old;
    }
    registerCreature(id, c);
    emit levelChanged();
}

void GameLevel::removeCreature(const QString& id) {
    Creature* c = m_creatures.take(id);
    if (!c) return;
    delete c;
    emit levelChanged();
}

void GameLevel::loadLevel1() {
    resetCreatures();
    Player* player = new Player(15, 3, this);
    Enemy* enemy = new Enemy(this);

    player->clearMethods();
    player->addMethod({ "void attack(Creature& target);", "player.attack(bug);" });
    player->setExtraCodeHtml(QString());
    player->setShowAttackImpl(true);  // attack 的函数实现只在第一关写出这一次

    enemy->clearMethods();
    enemy->setIntent(Creature::IntentAttack, enemy->atk());
    enemy->setDescription(QStringLiteral("很弱的敌人，生命值和攻击力都很低。"));

    registerCreature("player", player);
    registerCreature("bug", enemy);
    m_maxSteps = 5;
    m_currentLevel = 1;
    m_levelName = GameLevel::levelName(1);
    m_intro = m_levelName;
    emit levelChanged();
}

void GameLevel::loadLevel2() {
    resetCreatures();

    Player* player = new Player(15, 3, this);
    player->clearMethods();
    player->addMethod({ "void attack(Creature& target);", "player.attack(goblin);" });

    Enemy* e1 = new Enemy("goblin", "goblin", "Goblin", 3, 5, this);
    Enemy* e2 = new Enemy("darkknight", "darkknight", "Elite", 10, 4, this);

    e1->clearMethods();
    e2->clearMethods();
    e1->addMethod({ "void attack(Creature& target);", "goblin.attack(darkknight);" });
    e2->addMethod({ "void attack(Creature& target);", "darkknight.attack(goblin);" });

    // 保留 darkknight.attack(...) 的可点击尝试入口，用来引导玩家发现访问权限错误；
    // 运行时会在 CommandParser 中判定失败，不增长步数，也不触发怪物行动。
    e1->setExtraCodeHtml(QString());
    e2->setExtraCodeHtml(QString());
    e2->setMethodAccess(QStringLiteral("protected"));

    e1->setDescription(QStringLiteral("身板很脆，但它手里的匕首怎么那么好？！"));
    e2->setDescription(QStringLiteral("黑暗骑士，中等血量和较高的攻击。"));

    e1->setIntent(Creature::IntentAttack, e1->atk());
    e2->setIntent(Creature::IntentAttack, e2->atk());

    registerCreature("player", player);
    registerCreature("goblin", e1);
    registerCreature("darkknight", e2);
    m_maxSteps = 8;
    m_currentLevel = 2;
    m_levelName = GameLevel::levelName(2);
    m_allowEnemyControl = true;
    emit levelChanged();
}

void GameLevel::loadLevel3() {
    resetCreatures();

    Player* p = new Player(15, 3, this);
    p->clearMethods();
    p->setDef(3);
    p->addClassDeclaration("class Bomb;");
    p->addMethod({ "Bomb* summonBomb();", "player.summonBomb();" });
    p->addMethod({ "void attack(Creature& target);", "player.attack(darkknight);" });
    p->addMethod({ "void defend();", "player.defend();" });
    p->setDescription(QStringLiteral("玩家可以召唤 Bomb、普通攻击或防御。防御会生成等于当前 DEF 的护盾。"));
    p->setExtraCodeHtml(htmlBlock("Player::summonBomb / Player::defend",
        "Player::Bomb* Player::summonBomb() {\n"
        "    Player::Bomb* bomb = new Player::Bomb(*this);\n"
        "    world.add(bomb);\n"
        "    return bomb;\n"
        "}\n\n"
        "void Player::defend() {\n"
        "    shield = def;\n"
        "}\n\n"
        "void Creature::takeDamage(int damage) {\n"
        "    int absorbed = std::min(shield, damage);\n"
        "    shield -= absorbed;\n"
        "    hp -= damage - absorbed;\n"
        "}"));

    Enemy* e = new Enemy("darkknight", "darkknight", "Elite", 20, 3, this);
    e->clearMethods();
    e->addClassDeclaration("void attack(Creature& target);");
    e->addClassDeclaration("void empower();");
    e->setMethodAccess(QStringLiteral("protected"));
    e->setDescription(QStringLiteral("来复仇的黑暗骑士，可以强化自己获得超高攻击力！"));
    e->setExtraCodeHtml(htmlBlock("Elite::empower",
        "void Elite::empower() {\n"
        "    atk += 5;\n"
        "}"));
    e->setIntent(Creature::IntentBuff, 5);
    e->setIntentFunctionName(QStringLiteral("empower"));

    registerCreature("player", p);
    registerCreature("darkknight", e);
    m_maxSteps = 8;
    m_currentLevel = 3;
    m_levelName = GameLevel::levelName(3);
    m_intro = m_levelName;
    emit levelChanged();
}

void GameLevel::loadLevel4() {
    // 对象切片：ImmortalKing 重写 onHit 实现无敌，直接攻击无效。
    // Throne::seat(Creature c) 按值传参，会把 king 拷贝成一个普通 Creature，
    // 派生部分（含 onHit override）被切掉——无敌随之消失。
    // 对照的 Throne::crown(Creature& c) 按引用传参，多态保留，无敌不变。
    resetCreatures();
    Player* p = new Player(14, 3, this);

    Enemy* king = new Enemy(QStringLiteral("king"), QStringLiteral("Immortal King"), QStringLiteral("ImmortalKing"), 9, 3, this);
    king->clearMethods();
    king->addClassDeclaration(QStringLiteral("void onHit(Creature& attacker, int damage) override;"));
    king->setExtraCodeHtml(htmlBlock(QStringLiteral("ImmortalKing::onHit"),
        "void ImmortalKing::onHit(Creature& attacker, int damage) {\n"
        "    return;\n"
        "}\n"
        "// 王冠护体：onHit 被重写成「什么都不做」，吸收一切伤害，本体永不掉血。\n"
        "// 只要 king 仍是 ImmortalKing，直接攻击就完全无效。"));
    king->setIntent(Creature::IntentAttack, 3);
    king->setDescription(QStringLiteral("戴着王冠的不死之王。重写的 virtual onHit 吸收一切伤害，直接攻击无效。"));

    Creature* throne = new Creature(QStringLiteral("throne"), QStringLiteral("Throne"), QStringLiteral("Throne"), 1, 0, true, this);
    throne->addMethod({ QStringLiteral("void seat(Creature c);"), QStringLiteral("throne.seat(king);") });
    throne->addMethod({ QStringLiteral("void crown(Creature& c);"), QStringLiteral("throne.crown(king);") });
    throne->setMethodAccess(QStringLiteral("public"));
    throne->setExtraCodeHtml(htmlBlock(QStringLiteral("Throne::seat / place 的实际含义"),
        "void Throne::seat(Creature c) {\n"
        "    place(c);\n"
        "}\n"
        "// seat 的形参按值传递（没有 &）。进入函数时，ImmortalKing king 被拷贝成 Creature c，\n"
        "// 这一步就发生了对象切片：c 只剩基类 Creature 部分，onHit override 和王冠无敌都不在 c 里。\n"
        "// place(c) 再用这个普通 Creature 替换场上的 king，于是 king 变得可以正常受伤。\n"
        "\n"
        "void Throne::crown(Creature& c) {\n"
        "    place(c);\n"
        "}\n"
        "// crown 的形参按引用传递（有 &）。c 不是副本，而是原 king 的别名，仍指向真正的 ImmortalKing。\n"
        "// place(c) 放回的还是原对象，没有切片、无敌保留——所以 crown 不能用来破防。\n"
        "\n"
        "void place(CreatureLike c) {\n"
        "    world.remove(\"king\");\n"
        "    world.add(\"king\", c);\n"
        "}\n"
        "// place(c) 是本关的教学引擎函数，不是 C++ 标准库函数。它把参数 c 代表的对象放回 king 的位置，\n"
        "// 取代原来的 king。之后玩家再输入 player.attack(king)，打到的就是被 place(c) 放回去的新对象。\n"
        "// seat() 传进来的是已被切片的 Creature；crown() 传进来的是原对象引用，因此仍是 ImmortalKing。"));
    throne->setIntent(Creature::IntentUnknown, 0);
    throne->setDescription(QStringLiteral("王座（道具）。place(c) 会把当前 c 放回 king 的位置；seat(Creature c) 先制造被切片的 Creature 副本，crown(Creature& c) 只拿到原对象引用。"));

    registerCreature("player", p);
    registerCreature("king", king);
    registerCreature("throne", throne);

    m_maxSteps = 5;
    m_currentLevel = 4;
    m_levelName = GameLevel::levelName(4);
    m_intro = m_levelName;
    m_allowEnemyControl = true;
    emit levelChanged();
}

void GameLevel::loadLevel5() {
    // 静态狂暴（升级版）：sharedRage 是 Berserker 类的 static 数据成员，全类共享一份。
    // 玩家每攻击一次，sharedRage++，然后所有存活 Berserker 的 atk += sharedRage——
    // 越打怪物越疼，且伤害是「全类共享」地累加。static 成员函数 Berserker::calm() 把
    // sharedRage 清零，并将所有 Berserker 的攻击力恢复为原值。
    resetCreatures();
    Player* p = new Player(14, 3, this);

    auto makeBerserker = [this](const QString& id) {
        Enemy* b = new Enemy(id, QStringLiteral("Berserker"), QStringLiteral("Berserker"), 6, 1, this);
        b->clearMethods();
        b->addMethod({ QStringLiteral("static void calm();"), QStringLiteral("Berserker::calm();") });
        b->setMethodAccess(QStringLiteral("public"));
        b->setClassBodyNote(QStringLiteral("// sharedRage 属于 Berserker 类本身，所有实例共享同一份"));
        b->addClassDeclaration(QStringLiteral("static int sharedRage;"));
        b->addClassDeclaration(QStringLiteral("void enrage();   // 玩家每次攻击后被调用"));
        b->setExtraCodeHtml(htmlBlock(QStringLiteral("Berserker 的静态成员"),
            "int Berserker::sharedRage = 0;\n"
            "// 静态数据成员：在类外定义，整个 Berserker 类只有这一份，被所有实例共享。\n"
            "\n"
            "void Berserker::enrage() {\n"
            "    sharedRage++;\n"
            "    atk += sharedRage;\n"
            "}\n"
            "// 玩家每攻击一次，引擎就对所有存活的 Berserker 调用一次 enrage()：\n"
            "// sharedRage 先 +1（全类同步可见），每只狂战士再把它累加到自己的 atk 上——越打越疼。\n"
            "\n"
            "static void Berserker::calm() {\n"
            "    sharedRage = 0;\n"
            "}\n"
            "// 静态成员函数，用 Berserker::calm() 调用（不依附某个实例）。它把共享的 sharedRage 清零，\n"
            "// 引擎随后把所有 Berserker 的 atk 恢复为原始基础值。"));
        b->setIntent(Creature::IntentAttack, 1);
        b->setDescription(QStringLiteral("狂战士（基础 ATK 1）。你每攻击一次，sharedRage++，所有狂战士 atk += sharedRage——越打越疼。Berserker::calm() 可把全员攻击力清回原值。"));
        return b;
    };

    registerCreature("player", p);
    registerCreature("berserker1", makeBerserker("berserker1"));
    registerCreature("berserker2", makeBerserker("berserker2"));

    m_maxSteps = 7;
    m_currentLevel = 5;
    m_levelName = GameLevel::levelName(5);
    m_intro = m_levelName;
    m_allowEnemyControl = false;
    setCounter("level5_rage", 0);
    emit levelChanged();
}

void GameLevel::loadLevel6() {
    resetCreatures();
    Player* p = new Player(this);
    Enemy* boss = new Enemy("boss", "Shielded Boss", "ShieldedBoss", 18, 4, this);
    Enemy* bomb = new Enemy("bomb", "Volatile Bomb", "VolatileBomb", 1, 0, this);
    Enemy* stone = new Enemy("stone", "Stone Enemy", "StoneEnemy", 7, 0, this);

    // 解题关键是 virtual onHit 的分发：基类的 virtual onHit 与默认实现展示在 Creature 类中，
    // 每个子类各自把重载的 onHit「声明 + 具体实现」放在自己对应的源码区，而不是全堆在 boss 上。
    boss->clearMethods();
    boss->addClassDeclaration("void onHit(Creature& attacker, int damage) override;");
    boss->setExtraCodeHtml(htmlBlock("ShieldedBoss::onHit",
        "void ShieldedBoss::onHit(Creature& attacker, int damage) {\n"
        "    return;\n"
        "}\n"
        "// 护盾吸收普通攻击：onHit 被重写为直接返回，本体不掉血。"));

    stone->clearMethods();
    stone->addClassDeclaration("void onHit(Creature& attacker, int damage) override;");
    stone->setExtraCodeHtml(htmlBlock("StoneEnemy::onHit",
        "void StoneEnemy::onHit(Creature& attacker, int damage) {\n"
        "    attacker.takeDamage(3);\n"
        "}\n"
        "// 石化外壳：自身免疫伤害，并反弹 3 点给攻击者。"));

    bomb->clearMethods();
    bomb->addClassDeclaration("void onHit(Creature& attacker, int damage) override;");
    bomb->setExtraCodeHtml(htmlBlock("VolatileBomb::onHit",
        "void VolatileBomb::onHit(Creature& attacker, int damage) {\n"
        "    hp = 0;\n"
        "    boss.takeDamage(999);\n"
        "}\n"
        "// 受击立即引爆：清空自身，并把爆炸伤害传导给场上的 boss。"));

    bomb->setIntent(Creature::IntentUnknown, 0);
    stone->setIntent(Creature::IntentDefend, 0);
    boss->setIntent(Creature::IntentAttack, boss->atk());

    registerCreature("player", p);
    registerCreature("boss", boss);
    registerCreature("bomb", bomb);
    registerCreature("stone", stone);

    m_maxSteps = 6;
    m_currentLevel = 6;
    m_levelName = GameLevel::levelName(6);
    emit levelChanged();
}

void GameLevel::loadLevel7() {
    // 真身显形：dynamic_cast 向下转型（RTTI）。
    // mimic 的真实类型是 Mimic（派生类），但场上以 Creature& 基类视图交给玩家，
    // 它的弱点 revealWeakness() 是 Mimic 独有成员，基类视图里看不到、不能直接调用。
    // 伪装状态下 onHit 吸收一切伤害，直接攻击无效。鉴定师 appraiser 用 dynamic_cast<Mimic*>
    // 在运行期检查真实类型：是 Mimic 就拿到真身指针、调用 revealWeakness() 剥下伪装；
    // 不是 Mimic（如 decoy）则返回 nullptr，安全失败。
    resetCreatures();
    Player* p = new Player(14, 3, this);

    Enemy* mimic = new Enemy(QStringLiteral("mimic"), QStringLiteral("Mimic"), QStringLiteral("Mimic"), 8, 3, this);
    mimic->clearMethods();
    mimic->addClassDeclaration(QStringLiteral("void onHit(Creature& attacker, int damage) override;"));
    mimic->addClassDeclaration(QStringLiteral("void revealWeakness();   // Mimic 特有：基类 Creature 没有这个接口"));
    mimic->setExtraCodeHtml(htmlBlock(QStringLiteral("Mimic::onHit / revealWeakness"),
        "void Mimic::onHit(Creature& attacker, int damage) {\n"
        "    if (disguised) return;\n"
        "    hp -= damage;\n"
        "}\n"
        "// 伪装状态下重写的 onHit 直接返回，吸收一切伤害（和不死之王类似）。\n"
        "\n"
        "void Mimic::revealWeakness() {\n"
        "    disguised = false;\n"
        "}\n"
        "// revealWeakness() 是 Mimic 派生类独有的成员，基类 Creature 里没有它。\n"
        "// 场上对象以 Creature& 基类视图交给你，编译期看不到这个方法，因此不能写 mimic.revealWeakness()。\n"
        "// 必须先把基类引用向下转型成真正的 Mimic 才能调用它——这正是 dynamic_cast 的用途。"));
    mimic->setIntent(Creature::IntentAttack, 3);
    mimic->setDescription(QStringLiteral("拟态魔。真实类型是 Mimic，却以 Creature& 基类视图示人。伪装时 onHit 吸收一切伤害；弱点 revealWeakness() 是 Mimic 派生类独有的接口。"));

    Creature* appraiser = new Creature(QStringLiteral("appraiser"), QStringLiteral("Appraiser"), QStringLiteral("Appraiser"), 1, 0, true, this);
    appraiser->addMethod({ QStringLiteral("void reveal(Creature& c);"), QStringLiteral("appraiser.reveal(mimic);") });
    appraiser->setMethodAccess(QStringLiteral("public"));
    appraiser->setExtraCodeHtml(htmlBlock(QStringLiteral("Appraiser::reveal —— dynamic_cast 向下转型"),
        "void Appraiser::reveal(Creature& c) {\n"
        "    if (Mimic* m = dynamic_cast<Mimic*>(&c)) {\n"
        "        m->revealWeakness();\n"
        "    }\n"
        "}\n"
        "// dynamic_cast<Mimic*>(&c) 在运行期检查 c 的真实类型：\n"
        "//   · 真实类型是 Mimic → 返回有效指针，进入 if，调用派生类专属的 revealWeakness()。\n"
        "//   · 真实类型不是 Mimic（如 decoy）→ 返回 nullptr，安全跳过，不会产生未定义行为。\n"
        "// 这正是 dynamic_cast 相比 static_cast 的关键：运行期做类型检查，失配时返回空指针。"));
    appraiser->setIntent(Creature::IntentUnknown, 0);
    appraiser->setDescription(QStringLiteral("鉴定师（道具）。reveal(Creature& c) 内部用 dynamic_cast<Mimic*> 向下转型：命中真身才调用 revealWeakness()，类型不符则安全返回 nullptr。"));

    Creature* decoy = new Creature(QStringLiteral("decoy"), QStringLiteral("Decoy Chest"), QStringLiteral("DecoyChest"), 1, 0, true, this);
    decoy->clearMethods();
    decoy->setInheritInfo(QStringLiteral("Creature"), QStringLiteral("public"));
    decoy->setIntent(Creature::IntentUnknown, 0);
    decoy->setExtraCodeHtml(htmlBlock(QStringLiteral("Decoy 的真实类型"),
        "class DecoyChest : public Creature {\n"
        "};\n"
        "// decoy 的真实类型是 DecoyChest，并不是 Mimic。\n"
        "// 对它做 dynamic_cast<Mimic*> 会得到 nullptr——它用来演示「类型不符时安全失败」。"));
    decoy->setDescription(QStringLiteral("诱饵：一个伪装成 Mimic 的箱子，真实类型其实是 DecoyChest。对它 dynamic_cast<Mimic*> 会返回 nullptr。"));

    registerCreature("player", p);
    registerCreature("mimic", mimic);
    registerCreature("appraiser", appraiser);
    registerCreature("decoy", decoy);

    m_maxSteps = 6;
    m_currentLevel = 7;
    m_levelName = GameLevel::levelName(7);
    m_intro = m_levelName;
    m_allowEnemyControl = true;
    emit levelChanged();
}

void GameLevel::loadLevel8() {
    // 友元内鬼：Vault 的弱点 selfDestruct() 是 private，外部不能调用。
    // 但 Vault 在类体里声明了 friend class Saboteur，于是只有 Saboteur 能合法触发它。
    // Rogue 看起来也像帮手，却不是 friend，无法访问 private 成员。
    resetCreatures();
    Player* p = new Player(12, 3, this);

    Enemy* vault = new Enemy(QStringLiteral("vault"), QStringLiteral("Vault Boss"), QStringLiteral("Vault"), 30, 4, this);
    vault->clearMethods();
    vault->setMethodAccess(QStringLiteral("private"));
    vault->setClassBodyNote(QStringLiteral("friend class Saboteur;   // 只有 Saboteur 是金库的朋友"));
    vault->addClassDeclaration(QStringLiteral("void selfDestruct();   // private：外部不可调用"));
    vault->setExtraCodeHtml(htmlBlock(QStringLiteral("谁能触发 selfDestruct()"),
        "void Saboteur::betray(Vault& v) {\n"
        "    v.selfDestruct();\n"
        "}\n"
        "// 合法：Saboteur 是 Vault 声明的 friend，可以访问它的 private 成员 selfDestruct()。\n"
        "\n"
        "void Rogue::pick(Vault& v) {\n"
        "    // v.selfDestruct();\n"
        "}\n"
        "// 上面那行若取消注释会编译错误：Rogue 不是 Vault 的 friend，无法访问其 private 成员。"));
    vault->setIntent(Creature::IntentAttack, 4);
    vault->setDescription(QStringLiteral("坚不可摧的金库。常规攻击无效，弱点 selfDestruct() 是 private，只有它声明的 friend 才能触发。"));

    Creature* saboteur = new Creature(QStringLiteral("saboteur"), QStringLiteral("Saboteur"), QStringLiteral("Saboteur"), 1, 0, true, this);
    saboteur->addMethod({ QStringLiteral("void betray(Vault& v);"), QStringLiteral("saboteur.betray(vault);") });
    saboteur->setMethodAccess(QStringLiteral("public"));
    saboteur->setIntent(Creature::IntentUnknown, 0);
    saboteur->setDescription(QStringLiteral("内鬼。它在 Vault 的类体里被声明为 friend，因此能访问 Vault 的 private 成员。"));

    Creature* rogue = new Creature(QStringLiteral("rogue"), QStringLiteral("Rogue"), QStringLiteral("Rogue"), 1, 0, true, this);
    rogue->addMethod({ QStringLiteral("void pick(Vault& v);"), QStringLiteral("rogue.pick(vault);") });
    rogue->setMethodAccess(QStringLiteral("public"));
    rogue->setIntent(Creature::IntentUnknown, 0);
    rogue->setDescription(QStringLiteral("普通盗贼。没有出现在 Vault 的 friend 声明中，无法访问它的 private 成员。"));

    registerCreature("player", p);
    registerCreature("vault", vault);
    registerCreature("saboteur", saboteur);
    registerCreature("rogue", rogue);

    m_maxSteps = 5;
    m_currentLevel = 8;
    m_levelName = GameLevel::levelName(8);
    m_intro = m_levelName;
    m_allowEnemyControl = true;
    emit levelChanged();
}

void GameLevel::loadLevel9() {
    // 共享所有权：SoulGem 由若干 Cultist 各自持有一份 shared_ptr（owner），
    // Lich 只用 weak_ptr 观察（不计入 use_count）。命匣的「HP」就是它的 use_count，
    // 只有当所有 owner 都消失（use_count → 0）命匣才析构，Lich 的 weak_ptr 失效、才会破防。
    resetCreatures();
    Player* p = new Player(16, 3, this);

    Enemy* lich = new Enemy(QStringLiteral("lich"), QStringLiteral("Lich Boss"), QStringLiteral("Lich"), 9, 2, this);
    lich->clearMethods();
    lich->addClassDeclaration(QStringLiteral("weak_ptr<SoulGem> gem;   // 只观察，不拥有"));
    lich->addClassDeclaration(QStringLiteral("void onHit(Creature& attacker, int damage) override;"));
    lich->setExtraCodeHtml(htmlBlock(QStringLiteral("Lich 与 SoulGem"),
        "void Lich::onHit(Creature& attacker, int damage) {\n"
        "    if (!gem.expired()) return;\n"
        "    hp -= damage;\n"
        "}\n"
        "// gem 是 weak_ptr：只要命匣还有 shared_ptr 拥有者（use_count > 0），gem.expired() 为 false，\n"
        "// Lich 直接 return、免疫一切伤害。只有 use_count 归零、命匣析构之后，Lich 才会真正掉血。"));
    lich->setIntent(Creature::IntentAttack, 2);
    lich->setDescription(QStringLiteral("巫妖。它用 weak_ptr 观察 SoulGem：只要命匣还在（use_count > 0）就免疫一切伤害。"));

    Creature* gem = new Creature(QStringLiteral("gem"), QStringLiteral("Soul Gem"), QStringLiteral("SoulGem"), 3, 0, true, this);
    gem->setExtraCodeHtml(htmlBlock(QStringLiteral("SoulGem 的所有权（这里的 HP = use_count）"),
        "struct SoulGem {\n"
        "    ~SoulGem();\n"
        "};\n"
        "// 命匣本体。它在本关的「HP」被用来可视化 shared_ptr 的 use_count。\n"
        "\n"
        "class Cultist : public Creature {\n"
        "    shared_ptr<SoulGem> gem;\n"
        "};\n"
        "// 每个 Cultist 都「拥有」命匣的一份 shared_ptr，使 use_count += 1。\n"
        "// Lich 则只用 weak_ptr「观察」，不增加 use_count。\n"
        "// 只有当所有 shared_ptr 拥有者都消失（use_count → 0），SoulGem 才会析构。\n"
        "// 所以直接攻击命匣没用，必须先消灭所有持有者。"));
    gem->setIntent(Creature::IntentUnknown, 0);
    gem->setDescription(QStringLiteral("命匣。它的 HP 就是 SoulGem 的 use_count。每消灭一个 Cultist，use_count 减 1；归零时命匣析构。"));

    auto makeCultist = [this](const QString& id) {
        Enemy* c = new Enemy(id, QStringLiteral("Cultist"), QStringLiteral("Cultist"), 3, 1, this);
        c->clearMethods();
        c->addClassDeclaration(QStringLiteral("shared_ptr<SoulGem> gem;   // 拥有命匣的一份引用"));
        c->setIntent(Creature::IntentAttack, 1);
        c->setDescription(QStringLiteral("信徒，持有 SoulGem 的一份 shared_ptr。消灭它会让 use_count 减 1。"));
        return c;
    };

    registerCreature("player", p);
    registerCreature("lich", lich);
    registerCreature("gem", gem);
    registerCreature("cultist1", makeCultist("cultist1"));
    registerCreature("cultist2", makeCultist("cultist2"));
    registerCreature("cultist3", makeCultist("cultist3"));

    m_maxSteps = 7;
    m_currentLevel = 9;
    m_levelName = GameLevel::levelName(9);
    m_intro = m_levelName;
    m_allowEnemyControl = false;
    setCounter("level9_refcount", 3);
    emit levelChanged();
}

void GameLevel::loadLevel10() {
    // 菱形继承：Paladin : public Healer, public Warrior，二者各自继承 Unit 且未用虚继承。
    // 于是 Paladin 中有两个独立的 Unit 子对象（两条 hp：Healer::hp 与 Warrior::hp）。
    // 直接 paladin.expose(...) 有二义性；必须用作用域限定 paladin.Healer::expose / paladin.Warrior::expose
    // 分别清零两条 hp。Healer 半身每回合治疗 Warrior 子对象，所以要先清掉 Healer。
    resetCreatures();
    Player* p = new Player(14, 3, this);

    Enemy* paladin = new Enemy(QStringLiteral("paladin"), QStringLiteral("Paladin Boss"), QStringLiteral("Paladin"), 12, 0, this);
    paladin->clearMethods();
    paladin->setInheritInfo(QStringLiteral("Healer, public Warrior"), QStringLiteral("public"));
    paladin->setMethodAccess(QStringLiteral("public"));
    paladin->setClassBodyNote(QStringLiteral("// 菱形继承：Healer 与 Warrior 各自继承 Unit，且未使用虚继承"));
    paladin->addMethod({ QStringLiteral("void Healer::expose(Player& by);"), QStringLiteral("paladin.Healer::expose(player);") });
    paladin->addMethod({ QStringLiteral("void Warrior::expose(Player& by);"), QStringLiteral("paladin.Warrior::expose(player);") });
    paladin->addClassDeclaration(QStringLiteral("// void expose(Player& by);   // 直接调用二义，必须用 Base::expose 限定"));
    paladin->setExtraCodeHtml(htmlBlock(QStringLiteral("菱形继承结构"),
        "class Unit {\n"
        "protected:\n"
        "    int hp;\n"
        "};\n"
        "// 公共基类，持有 hp。\n"
        "\n"
        "class Healer : public Unit {\n"
        "public:\n"
        "    void expose(Player& by);\n"
        "};\n"
        "// Healer 半身：每回合治疗 Warrior 子对象。\n"
        "\n"
        "class Warrior : public Unit {\n"
        "public:\n"
        "    void expose(Player& by);\n"
        "};\n"
        "// Warrior 半身：每回合攻击 player。\n"
        "\n"
        "class Paladin : public Healer, public Warrior {\n"
        "};\n"
        "// 未使用虚继承：Paladin 里有「两个」独立的 Unit 子对象，因此有两条 hp\n"
        "//（Healer::hp 与 Warrior::hp），必须分别清零。调用方式：\n"
        "//   paladin.expose(player);          → 二义，编译失败（两个 expose / 两个 Unit::hp）\n"
        "//   paladin.Healer::expose(player);  → 作用域限定，选中 Healer 子对象\n"
        "//   paladin.Warrior::expose(player); → 选中 Warrior 子对象"));
    paladin->setIntent(Creature::IntentNone, 0);   // 行为由 GameController 自定义
    paladin->setDescription(QStringLiteral("圣骑士。Healer 半身每回合治疗 Warrior 子对象，Warrior 半身每回合攻击你。两个 Unit 子对象各有一条 HP（各 6），必须用 Base::expose 分别清零。"));

    registerCreature("player", p);
    registerCreature("paladin", paladin);

    m_maxSteps = 7;
    m_currentLevel = 10;
    m_levelName = GameLevel::levelName(10);
    m_intro = m_levelName;
    m_allowEnemyControl = true;
    setCounter("level10_healer", 6);
    setCounter("level10_warrior", 6);
    emit levelChanged();
}

Creature* GameLevel::creature(const QString& id) const { return m_creatures.value(id, nullptr); }
QList<Creature*> GameLevel::creatures() const { return m_creatures.values(); }

int GameLevel::usedSteps() const { return m_usedSteps; }
int GameLevel::maxSteps() const { return m_maxSteps; }
int GameLevel::remainingSteps() const { return qMax(0, m_maxSteps - m_usedSteps); }
int GameLevel::currentLevel() const { return m_currentLevel; }
QString GameLevel::levelName() const { return m_levelName.isEmpty() ? GameLevel::levelName(m_currentLevel) : m_levelName; }
bool GameLevel::allowEnemyControl() const { return m_allowEnemyControl; }
QString GameLevel::intro() const { return m_intro; }

QString GameLevel::hintForCurrentLevel() const {
    switch (m_currentLevel) {
    case 1: return "尝试最直接的 public 接口。";
    case 2: return "敌人也能成为其他敌人的目标。";
    case 3: return "有些伤害会在作用域结束时结算。";
    case 4: return "按值传参会切掉派生部分；注意形参有没有 &。";
    case 5: return "越打它们越疼，且伤害全类共享——该让全类冷静了。";
    case 6: return "不同对象收到同一个攻击时，结果不一定一样。";
    case 7: return "直接打它会被伪装吸收；先用 dynamic_cast 把基类引用向下转型成 Mimic（appraiser.reveal(mimic)）。";
    case 8: return "私有弱点只有 friend 能碰——谁是 Vault 的 friend？";
    case 9: return "命匣的寿命由 shared_ptr 的引用计数决定，先清空所有 owner。";
    case 10: return "有两个 Unit 子对象；用 Base::expose 分别处理，注意先后。";
    default: return "阅读右侧源码。";
    }
}

void GameLevel::consumeStep() {
    ++m_usedSteps;
    emit levelChanged();
}

void GameLevel::setFlag(const QString& key, bool value) {
    m_flags.insert(key, value);
    emit levelChanged();
}

void GameLevel::setCounter(const QString& key, int value) {
    m_counters.insert(key, value);
    emit levelChanged();
}

int GameLevel::counter(const QString& key, int defaultValue) const {
    return m_counters.value(key, defaultValue);
}

void GameLevel::removeCounter(const QString& key) {
    if (!m_counters.contains(key)) return;
    m_counters.remove(key);
    emit levelChanged();
}

bool GameLevel::flag(const QString& key) const { return m_flags.value(key, false); }
void GameLevel::clearFlags() {
    m_flags.clear();
    emit levelChanged();
}

bool GameLevel::isWin() const {
    bool anyEnemy = false;
    for (Creature* c : m_creatures) {
        if (!c || !c->isEnemy()) continue;
        // 第六关：stone 是用来演示 onHit 反弹/免疫的教学敌人，本身无法被击杀，不计入全灭判定。
        if (m_currentLevel == 6 && c->id() == "stone") {
            continue;
        }
        // 第四关：throne 是切片道具，不是要击杀的敌人。
        if (m_currentLevel == 4 && c->id() == "throne") {
            continue;
        }
        // 第七关：appraiser（鉴定师）与 decoy（诱饵）是道具型对象，不计入全灭。
        if (m_currentLevel == 7 && (c->id() == "appraiser" || c->id() == "decoy")) {
            continue;
        }
        // 第八关：saboteur / rogue 是道具型对象，不计入全灭。
        if (m_currentLevel == 8 && (c->id() == "saboteur" || c->id() == "rogue")) {
            continue;
        }
        // 第九关：gem（SoulGem）是共享资源，它的 HP 表示 use_count，不计入全灭。
        if (m_currentLevel == 9 && c->id() == "gem") {
            continue;
        }
        anyEnemy = true;
        if (c->isAlive()) return false;
    }
    return anyEnemy;
}

bool GameLevel::isLose() const {
    Creature* player = creature("player");
    if (!player || !player->isAlive()) return true;
    return m_usedSteps >= m_maxSteps && !isWin();
}
