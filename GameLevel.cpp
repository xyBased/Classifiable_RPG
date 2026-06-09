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
    case 7: return QStringLiteral("建设中");
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
        "    return;   // 王冠护体：吸收一切伤害，本体永不掉血\n"
        "}"));
    king->setIntent(Creature::IntentAttack, 3);
    king->setDescription(QStringLiteral("戴着王冠的不死之王。重写的 virtual onHit 吸收一切伤害，直接攻击无效。"));

    Creature* throne = new Creature(QStringLiteral("throne"), QStringLiteral("Throne"), QStringLiteral("Throne"), 1, 0, true, this);
    throne->addMethod({ QStringLiteral("void seat(Creature c);"), QStringLiteral("throne.seat(king);") });
    throne->addMethod({ QStringLiteral("void crown(Creature& c);"), QStringLiteral("throne.crown(king);") });
    throne->setMethodAccess(QStringLiteral("public"));
    throne->setExtraCodeHtml(htmlBlock(QStringLiteral("Throne::seat / place 的实际含义"),
        "// place(c) 是本关的教学引擎函数，不是 C++ 标准库函数。\n"
        "// 它做的事是：把当前参数 c 所代表的对象放回场上 king 的位置，\n"
        "// 取代原来的 king。之后玩家继续输入 player.attack(king)，\n"
        "// 打到的就是被 place(c) 放回去的新对象。\n\n"
        "void Throne::seat(Creature c) {     // 形参按值传递（没有 &）\n"
        "    // 进入函数前，ImmortalKing king 被拷贝到 Creature c。\n"
        "    // 这一步已经发生对象切片：c 只剩 Creature 基类部分，\n"
        "    // ImmortalKing::onHit override 和王冠无敌都不在 c 里。\n"
        "    place(c);   // 用这个普通 Creature c 替换场上的 king\n"
        "}\n\n"
        "void Throne::crown(Creature& c) {   // 形参按引用传递（有 &）\n"
        "    // c 不是副本，而是原 king 的别名，仍指向真正的 ImmortalKing。\n"
        "    place(c);   // 放回的仍是原 ImmortalKing，本体未切片，无敌保留\n"
        "}\n\n"
        "// 教学伪代码：展示 place(c) 在游戏引擎中的效果\n"
        "void place(/* 当前参数对象 */ CreatureLike c) {\n"
        "    world.remove(\"king\");\n"
        "    world.add(\"king\", copy_of_current_object(c));\n"
        "    // seat() 传来的是已切片的 Creature；\n"
        "    // crown() 传来的是原对象引用，因此仍是 ImmortalKing。\n"
        "}"));
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
            "int Berserker::sharedRage = 0;   // 静态数据成员：在类外定义，全类只有一份\n\n"
            "// 玩家每攻击一次，引擎对所有存活 Berserker 调用 enrage()：\n"
            "void Berserker::enrage() {\n"
            "    sharedRage++;        // 共享计数 +1（全类可见、全类同步）\n"
            "    atk += sharedRage;   // 攻击力叠加当前 sharedRage —— 越打越疼\n"
            "}\n\n"
            "static void Berserker::calm() {   // 静态成员函数：用 Berserker::calm() 调用\n"
            "    sharedRage = 0;               // 清零共享计数\n"
            "    // 同时把所有 Berserker 的 atk 恢复为原值\n"
            "}"));
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
        "    // 护盾吸收普通攻击，本体不掉血\n"
        "    return;\n"
        "}"));

    stone->clearMethods();
    stone->addClassDeclaration("void onHit(Creature& attacker, int damage) override;");
    stone->setExtraCodeHtml(htmlBlock("StoneEnemy::onHit",
        "void StoneEnemy::onHit(Creature& attacker, int damage) {\n"
        "    // 石化外壳：自身免疫伤害，并反弹 3 点给攻击者\n"
        "    attacker.takeDamage(3);\n"
        "}"));

    bomb->clearMethods();
    bomb->addClassDeclaration("void onHit(Creature& attacker, int damage) override;");
    bomb->setExtraCodeHtml(htmlBlock("VolatileBomb::onHit",
        "void VolatileBomb::onHit(Creature& attacker, int damage) {\n"
        "    // 受击立即引爆：清空自身，并把爆炸伤害传导给场上的 boss\n"
        "    hp = 0;\n"
        "    boss.takeDamage(999);\n"
        "}"));

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
    // 占位关（建设中）：本关暂时空着。放一个训练假人，玩家随手清掉即可继续，
    // 以免卡住关卡进度。后续可在此处填入新的知识点关卡。
    resetCreatures();
    Player* p = new Player(this);

    Enemy* dummy = new Enemy(QStringLiteral("dummy"), QStringLiteral("Training Dummy"), QStringLiteral("TrainingDummy"), 1, 0, this);
    dummy->clearMethods();
    dummy->setIntent(Creature::IntentDefend, 0);
    dummy->setDescription(QStringLiteral("训练假人。第 7 关正在建设中——这里暂时只有一个假人，攻击它即可通关，继续后面的关卡。"));
    dummy->setExtraCodeHtml(htmlBlock(QStringLiteral("建设中"),
        "// 第 7 关正在设计中（占位关）。\n"
        "// 攻击 dummy 即可过关，先去体验后面的关卡吧。"));

    registerCreature("player", p);
    registerCreature("dummy", dummy);

    m_maxSteps = 5;
    m_currentLevel = 7;
    m_levelName = GameLevel::levelName(7);
    m_intro = m_levelName;
    m_allowEnemyControl = false;
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
        "    v.selfDestruct();   // 合法：Saboteur 是 Vault 的 friend，可访问其 private 成员\n"
        "}\n\n"
        "void Rogue::pick(Vault& v) {\n"
        "    // v.selfDestruct();   // 编译错误：Rogue 不是 friend，无法访问 private\n"
        "}"));
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
        "    if (!gem.expired()) return;   // 命匣仍有 owner，Lich 免疫\n"
        "    hp -= damage;                 // use_count 归零、命匣析构后才会受伤\n"
        "}"));
    lich->setIntent(Creature::IntentAttack, 2);
    lich->setDescription(QStringLiteral("巫妖。它用 weak_ptr 观察 SoulGem：只要命匣还在（use_count > 0）就免疫一切伤害。"));

    Creature* gem = new Creature(QStringLiteral("gem"), QStringLiteral("Soul Gem"), QStringLiteral("SoulGem"), 3, 0, true, this);
    gem->setExtraCodeHtml(htmlBlock(QStringLiteral("SoulGem 的所有权（这里的 HP = use_count）"),
        "struct SoulGem { ~SoulGem(); };\n\n"
        "// 每个 Cultist 都「拥有」命匣的一份 shared_ptr：\n"
        "class Cultist : public Creature {\n"
        "    shared_ptr<SoulGem> gem;   // 拥有者，use_count += 1\n"
        "};\n\n"
        "// Lich 只「观察」，持有 weak_ptr，不增加 use_count。\n"
        "// 只有当所有 shared_ptr 拥有者都消失（use_count → 0），\n"
        "// SoulGem 才会析构。直接攻击命匣是没用的。"));
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
        "class Unit { protected: int hp; };   // 公共基类\n\n"
        "class Healer  : public Unit { public: void expose(Player& by); };   // 会治疗 Warrior 子对象\n"
        "class Warrior : public Unit { public: void expose(Player& by); };   // 会攻击 player\n\n"
        "// 未使用虚继承：Paladin 里有「两个」独立的 Unit 子对象，\n"
        "// 因此有两条 hp（Healer::hp 与 Warrior::hp），必须分别清零。\n"
        "class Paladin : public Healer, public Warrior {};\n\n"
        "// paladin.expose(player);            // 二义：两个 expose / 两个 Unit::hp\n"
        "// paladin.Healer::expose(player);    // 作用域限定，选中 Healer 子对象\n"
        "// paladin.Warrior::expose(player);   // 选中 Warrior 子对象"));
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
    case 7: return "（占位关）随手清掉训练假人即可继续。";
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
