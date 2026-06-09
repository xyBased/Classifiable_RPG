#include "GameLevel.h"

#include <initializer_list>
#include <QStringList>
#include <QtAlgorithms>

namespace {

<<<<<<< Updated upstream
QString compactCommand(QString command) {
    command.remove(' ');
    command.remove('\t');
    command.remove('\n');
    command.remove('\r');
    return command;
=======
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
>>>>>>> Stashed changes
}

QString linkCmd(const QString& command, const QString& shownText) {
    const QString compact = compactCommand(command);
    return QString("<a href=\"cmd:%1\" style=\"color:#89DDFF; text-decoration:underline; font-weight:700;\">%2</a>")
        .arg(compact.toHtmlEscaped(), shownText.toHtmlEscaped());
}

void addLine(QString& body, const QString& line = QString()) {
    body += line.toHtmlEscaped();
    body += "\n";
}

void addRawLine(QString& body, const QString& raw) {
    body += raw;
    body += "\n";
}

QString codeBlock(const QString& body) {
    return QString(
               "<pre style='font-family:Consolas, \"Courier New\", monospace;"
               "font-size:14px; line-height:1.35; color:#EAF1FF; margin:0;"
               "white-space: pre-wrap; overflow-wrap:anywhere; word-break:break-word;'>%1</pre>"
               ).arg(body);
}

Creature* makeActor(const QString& id,
                    const QString& name,
                    const QString& className,
                    int hp,
                    int atk,
                    bool countsForWin,
                    bool takesTurn,
                    const QString& campText,
                    QObject* parent) {
    Creature* c = new Creature(id, name, className, hp, atk, true, parent);
    c->setCountsForWin(countsForWin);
    c->setTakesTurn(takesTurn);
    c->setCampText(campText);
    return c;
}

QString defaultPlayerCode(const QString& target = QStringLiteral("enemy")) {
    QString b;
    addLine(b, "class Player : public Creature {");
    addLine(b, "public:");
    addRawLine(b, QStringLiteral("    ") + linkCmd(QString("player.attack(%1);").arg(target), "void attack(Creature& enemy);") );
    addRawLine(b, QStringLiteral("    ") + linkCmd(QString("player.powerAttack(%1);").arg(target), "void powerAttack(Creature& enemy);") );
    addRawLine(b, QStringLiteral("    ") + linkCmd("player.heal();", "void heal();") );
    addLine(b, "};");
    return codeBlock(b);
}

QString level3PlayerCode() {
    QString b;
    addLine(b, "class ScopedBomb {");
    addLine(b, "private:");
    addLine(b, "    Creature& target;");
    addLine(b);
    addLine(b, "public:");
    addLine(b, "    ScopedBomb(Creature& t) : target(t) {");
    addLine(b, "        // 构造时只是绑定目标");
    addLine(b, "    }");
    addLine(b);
    addLine(b, "    ~ScopedBomb() {");
    addLine(b, "        target.takeDamage(999);");
    addLine(b, "    }");
    addLine(b, "};");
    addLine(b);
    addLine(b, "class Player : public Creature {");
    addLine(b, "public:");
    addRawLine(b, QStringLiteral("    ") + linkCmd("player.plantBomb(enemy);", "void plantBomb(Creature& enemy)") + " {");
    addLine(b, "        ScopedBomb bomb(enemy);");
    addLine(b, "    } // bomb 在这里离开作用域，自动析构");
    addLine(b, "};");
    return codeBlock(b);
}

QString level3EnemyCode() {
    QString b;
    addLine(b, "class IronWall : public Enemy {");
    addLine(b, "public:");
    addLine(b, "    void takeDamage(int damage) {");
    addLine(b, "        if (damage < 999) return;");
    addLine(b, "        hp -= damage;");
    addLine(b, "    }");
    addLine(b, "};");
    return codeBlock(b);
}

QString level4GoblinCode() {
    QString b;
    addLine(b, "class Enemy : public Creature {");
    addLine(b, "public:");
    addLine(b, "    void attack(Creature& target);");
    addLine(b, "};");
    addLine(b);
    addLine(b, "class Goblin : public Enemy {");
    addLine(b, "    // 没有重新声明 attack()");
    addLine(b, "    // 但 public 继承保留了 Enemy 的 public 接口");
    addLine(b, "};");
    addLine(b);
    addRawLine(b, QStringLiteral("// 继承来的可访问接口：") + linkCmd("goblin.attack(boss);", "goblin.attack(boss);") );
    return codeBlock(b);
}

QString level4MimicCode() {
    QString b;
    addLine(b, "class Enemy : public Creature {");
    addLine(b, "public:");
    addLine(b, "    void attack(Creature& target);");
    addLine(b, "};");
    addLine(b);
    addLine(b, "class Mimic : private Enemy {");
    addLine(b, "    // private 继承会把 Enemy 的 public 成员变成 private");
    addLine(b, "};");
    addLine(b);
    addRawLine(b, QStringLiteral("// 试试看：") + linkCmd("mimic.attack(boss);", "mimic.attack(boss);") );
    return codeBlock(b);
}

QString level4SkeletonCode() {
    QString b;
    addLine(b, "class Enemy : public Creature {");
    addLine(b, "public:");
    addLine(b, "    void attack(Creature& target);");
    addLine(b, "};");
    addLine(b);
    addLine(b, "class Skeleton : protected Enemy {");
    addLine(b, "    // protected 继承让外部不能通过 Skeleton 对象访问 attack()");
    addLine(b, "};");
    addLine(b);
    addRawLine(b, QStringLiteral("// 试试看：") + linkCmd("skeleton.attack(boss);", "skeleton.attack(boss);") );
    return codeBlock(b);
}

QString level5PlayerCode() {
    QString b;
    addLine(b, "class Player : public Creature {");
    addLine(b, "public:");
    addLine(b, "    Player operator+(const Gem& gem) const {");
    addLine(b, "        Player temp = *this;");
    addLine(b, "        temp.atk += gem.power();");
    addLine(b, "        return temp; // 返回临时对象，不改原 player");
    addLine(b, "    }");
    addLine(b);
    addLine(b, "    Player& operator+=(const Gem& gem) {");
    addLine(b, "        this->atk += gem.power();");
    addLine(b, "        return *this; // 修改当前对象");
    addLine(b, "    }");
    addLine(b);
    addRawLine(b, QStringLiteral("    ") + linkCmd("player + ruby;", "player + ruby;") );
    addRawLine(b, QStringLiteral("    ") + linkCmd("player += ruby;", "player += ruby;") );
    addRawLine(b, QStringLiteral("    ") + linkCmd("player.attack(enemy);", "void attack(Creature& enemy);") );
    addLine(b, "};");
    return codeBlock(b);
}

QString level6PlayerCode() {
    QString b;
    addLine(b, "class Player : public Creature {");
    addLine(b, "public:");
    addLine(b, "    void attack(Creature& target) {");
    addLine(b, "        target.onHit(*this); // onHit 是 virtual");
    addLine(b, "    }");
    addLine(b);
    addRawLine(b, QStringLiteral("    ") + linkCmd("player.attack(boss);", "player.attack(boss);") );
    addRawLine(b, QStringLiteral("    ") + linkCmd("player.attack(stone);", "player.attack(stone);") );
    addRawLine(b, QStringLiteral("    ") + linkCmd("player.attack(bomb);", "player.attack(bomb);") );
    addLine(b, "};");
    return codeBlock(b);
}

QString level7PlayerCode() {
    QString b;
    addLine(b, "class Spell {");
    addLine(b, "public:");
    addLine(b, "    virtual void cast(Creature& target) = 0;");
    addLine(b, "};");
    addLine(b);
    addLine(b, "Spell* slot1 = new FireSpell();");
    addLine(b, "Spell* slot2 = new SilenceSpell();");
    addLine(b, "Spell* slot3 = new MirrorSpell();");
    addLine(b);
    addRawLine(b, linkCmd("slot1.cast(boss);", "slot1.cast(boss);") + "  // 静态类型 Spell*，实际类型 FireSpell");
    addRawLine(b, linkCmd("slot2.cast(boss);", "slot2.cast(boss);") + "  // 静态类型 Spell*，实际类型 SilenceSpell");
    addRawLine(b, linkCmd("slot3.cast(boss);", "slot3.cast(boss);") + "  // 静态类型 Spell*，实际类型 MirrorSpell");
    return codeBlock(b);
}

QString level8PlayerCode() {
    QString b;
    addLine(b, "template <typename Item>");
    addLine(b, "void use(Item& item, Creature& target) {");
    addLine(b, "    item.apply(target);");
    addLine(b, "}");
    addLine(b);
    addRawLine(b, linkCmd("use(healPotion, enemy);", "use(healPotion, enemy);") + "   // Item 推导为 HealPotion");
    addRawLine(b, linkCmd("use(poisonBomb, enemy);", "use(poisonBomb, enemy);") + "  // Item 推导为 PoisonBomb");
    return codeBlock(b);
}

QString level9PlayerCode() {
    QString b;
    addLine(b, "template <typename T>");
    addLine(b, "class Box {");
    addLine(b, "private:");
    addLine(b, "    T item;");
    addLine(b, "public:");
    addLine(b, "    T take() { return item; }");
    addLine(b, "};");
    addLine(b);
    addLine(b, "Box<HealPotion> box1;");
    addLine(b, "Box<PoisonBomb> box2;");
    addLine(b, "Box<Key> box3;");
    addLine(b);
    addRawLine(b, linkCmd("box3.take();", "box3.take();") + "       // 取出 Key");
    addRawLine(b, linkCmd("door.open(key);", "door.open(key);") + "   // 门只接受 Key");
    addRawLine(b, linkCmd("box2.take();", "box2.take();") + "       // 取出 PoisonBomb");
    addRawLine(b, linkCmd("use(poisonBomb, enemy);", "use(poisonBomb, enemy);") );
    return codeBlock(b);
}

QString level10PlayerCode() {
    QString b;
    addLine(b, "// 终局没有新语法，只有前面规则的组合。");
    addLine(b, "class Player : public Creature {");
    addLine(b, "public:");
    addRawLine(b, QStringLiteral("    ") + linkCmd("mimic.open();", "mimic.open();") + "          // public 继承来的接口");
    addRawLine(b, QStringLiteral("    ") + linkCmd("player + ruby;", "player + ruby;") + "        // 临时对象陷阱");
    addRawLine(b, QStringLiteral("    ") + linkCmd("player += ruby;", "player += ruby;") + "       // operator+= 修改本体");
    addRawLine(b, QStringLiteral("    ") + linkCmd("slot.cast(boss);", "slot.cast(boss);") + "       // 多态法术封印 Boss");
    addRawLine(b, QStringLiteral("    ") + linkCmd("player.attack(bomb);", "player.attack(bomb);") + "    // virtual onHit 触发炸弹");
    addRawLine(b, QStringLiteral("    ") + linkCmd("swapHp(weakDragon, boss);", "swapHp(weakDragon, boss);") + " // 函数模板要求同类型");
    addRawLine(b, QStringLiteral("    ") + linkCmd("player.plantBomb(boss);", "player.plantBomb(boss);") + "  // 析构函数收尾");
    addLine(b, "};");
    return codeBlock(b);
}

QString simpleCodeWithLines(std::initializer_list<QString> lines) {
    QString b;
    for (const QString& line : lines) {
        addLine(b, line);
    }
    return codeBlock(b);
}

QString rubyCode() {
    QString b;
    addLine(b, "class Gem {");
    addLine(b, "public:");
    addLine(b, "    int power() const { return 7; }");
    addLine(b, "};");
    addLine(b);
    addRawLine(b, linkCmd("player += ruby;", "player += ruby;") );
    return codeBlock(b);
}

QString level10MimicCode() {
    QString b;
    addLine(b, "class Chest {");
    addLine(b, "public:");
    addLine(b, "    void open();");
    addLine(b, "};");
    addLine(b);
    addLine(b, "class Mimic : public Chest {");
    addLine(b, "    // open() 是从 Chest public 继承来的接口");
    addLine(b, "};");
    addLine(b);
    addRawLine(b, linkCmd("mimic.open();", "mimic.open();") );
    return codeBlock(b);
}

} // namespace

GameLevel::GameLevel(QObject* parent)
    : QObject(parent) {}

GameLevel::~GameLevel() {
    qDeleteAll(m_creatures);
    m_creatures.clear();
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
    m_usedSteps = 0;
    m_allowEnemyControl = false;
}

void GameLevel::registerCreature(const QString& id, Creature* c) {
    if (!c) return;
    m_creatures.insert(id, c);
    connect(c, &Creature::changed, this, &GameLevel::levelChanged);
}

void GameLevel::loadLevel1() {
    resetCreatures();

    Player* player = new Player(this);
    Enemy* enemy = new Enemy(this);

    registerCreature("player", player);
    registerCreature("enemy", enemy);

    m_maxSteps = 5;
    m_currentLevel = 1;
    m_allowEnemyControl = false;
    m_intro =
        "关卡 1：对象与成员函数。\n"
        "目标：在 5 步内击败 enemy。\n"
        "读右侧 Player 类，点击或输入 player.attack(enemy);。";

    emit levelChanged();
}

void GameLevel::loadLevel2() {
    resetCreatures();

    Player* player = new Player(this);
    Enemy* e1 = new Enemy("enemy1", "Goblin", 3, 7, this);
    Enemy* e2 = new Enemy("enemy2", "Slime King", 7, 20, this);

    e1->addMethod({ "void attack(Creature& target);", "enemy1.attack(enemy2);" });
    e2->addMethod({ "void attack(Creature& target);", "enemy2.attack(enemy1);" });

    registerCreature("player", player);
    registerCreature("enemy1", e1);
    registerCreature("enemy2", e2);

    m_maxSteps = 8;
    m_currentLevel = 2;
    m_allowEnemyControl = true;
    m_intro =
        "关卡 2：public 访问控制。\n"
        "正常硬拼会被 enemy2 秒杀。\n"
        "提示：Enemy::attack() 在 public 区域，所以外部代码也能让 enemy1 攻击 enemy2。";

    emit levelChanged();
}

void GameLevel::loadLevel3() {
    resetCreatures();

    Player* player = new Player(this);
    player->setCustomClassCodeHtml(level3PlayerCode());

    Creature* enemy = makeActor("enemy", "Iron Wall", "IronWall", 10, 0, true, false, "Enemy", this);
    enemy->setCustomClassCodeHtml(level3EnemyCode());

    registerCreature("player", player);
    registerCreature("enemy", enemy);

    m_maxSteps = 2;
    m_currentLevel = 3;
    m_intro =
        "关卡 3：构造函数、析构函数与对象生命周期。\n"
        "IronWall 会忽略普通伤害。\n"
        "观察 Player::plantBomb 里的局部对象：真正的攻击藏在 ~ScopedBomb() 中。";

    emit levelChanged();
}

void GameLevel::loadLevel4() {
    // 对象切片：ImmortalKing 重写 onHit 实现无敌，直接攻击无效。
    // Throne::seat(Creature c) 按值传参，会把 king 拷贝成一个普通 Creature，
    // 派生部分（含 onHit override）被切掉——无敌随之消失。
    // 对照的 Throne::crown(Creature& c) 按引用传参，多态保留，无敌不变。
    resetCreatures();
<<<<<<< Updated upstream

    Player* player = new Player(this);
    player->setCustomClassCodeHtml(defaultPlayerCode(QStringLiteral("boss")));

    Creature* boss = makeActor("boss", "Private Boss", "InheritanceBoss", 12, 0, true, false, "Enemy", this);
    boss->setCustomClassCodeHtml(simpleCodeWithLines({
        "class InheritanceBoss : public Enemy {",
        "public:",
        "    void takeDamage(int damage) { hp -= damage; }",
        "};"
    }));

    Creature* goblin = makeActor("goblin", "Public Goblin", "Goblin", 1, 12, false, false, "Puzzle Object", this);
    goblin->setCustomClassCodeHtml(level4GoblinCode());

    Creature* mimic = makeActor("mimic", "Private Mimic", "Mimic", 1, 12, false, false, "Puzzle Object", this);
    mimic->setCustomClassCodeHtml(level4MimicCode());

    Creature* skeleton = makeActor("skeleton", "Protected Skeleton", "Skeleton", 1, 12, false, false, "Puzzle Object", this);
    skeleton->setCustomClassCodeHtml(level4SkeletonCode());

    registerCreature("player", player);
    registerCreature("boss", boss);
    registerCreature("goblin", goblin);
    registerCreature("mimic", mimic);
    registerCreature("skeleton", skeleton);

    m_maxSteps = 3;
    m_currentLevel = 4;
    m_intro =
        "关卡 4：继承与派生。\n"
        "Boss 需要 12 点伤害，玩家攻击力只有 3。\n"
        "三个小怪都像 Enemy，但只有 public 继承能把 Enemy::attack() 暴露给外部。";

=======
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
>>>>>>> Stashed changes
    emit levelChanged();
}

void GameLevel::loadLevel5() {
    // 静态狂暴（升级版）：sharedRage 是 Berserker 类的 static 数据成员，全类共享一份。
    // 玩家每攻击一次，sharedRage++，然后所有存活 Berserker 的 atk += sharedRage——
    // 越打怪物越疼，且伤害是「全类共享」地累加。static 成员函数 Berserker::calm() 把
    // sharedRage 清零，并将所有 Berserker 的攻击力恢复为原值。
    resetCreatures();
<<<<<<< Updated upstream

    Player* player = new Player(this);
    player->setCustomClassCodeHtml(level5PlayerCode());

    Creature* enemy = makeActor("enemy", "Armor Boss", "ArmorBoss", 10, 0, true, false, "Enemy", this);
    enemy->setCustomClassCodeHtml(simpleCodeWithLines({
        "class ArmorBoss : public Enemy {",
        "public:",
        "    void takeDamage(int damage) {",
        "        if (damage < 10) return;",
        "        hp -= damage;",
        "    }",
        "};"
    }));

    Creature* ruby = makeActor("ruby", "Ruby Gem", "Gem", 1, 0, false, false, "Item", this);
    ruby->setCustomClassCodeHtml(rubyCode());
    ruby->setExtraPropertyText("power：7\n提示：operator+ 和 operator+= 的效果不同。");

    registerCreature("player", player);
    registerCreature("enemy", enemy);
    registerCreature("ruby", ruby);

    m_maxSteps = 3;
    m_currentLevel = 5;
    m_intro =
        "关卡 5：运算符重载。\n"
        "ArmorBoss 会忽略小于 10 的伤害。\n"
        "注意：operator+ 返回临时对象，operator+= 才会修改 player 本体。";

=======
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
>>>>>>> Stashed changes
    emit levelChanged();
}

void GameLevel::loadLevel6() {
    resetCreatures();

    Player* player = new Player(this);
    player->setCustomClassCodeHtml(level6PlayerCode());

    Creature* boss = makeActor("boss", "Shielded Boss", "ShieldedBoss", 20, 0, true, false, "Enemy", this);
    boss->setCustomClassCodeHtml(simpleCodeWithLines({
        "class ShieldedBoss : public Creature {",
        "public:",
        "    void onHit(Creature& attacker) override {",
        "        // 护盾吸收普通攻击",
        "    }",
        "};"
    }));

    Creature* stone = makeActor("stone", "Stone Enemy", "StoneEnemy", 4, 0, false, false, "Puzzle Object", this);
    stone->setCustomClassCodeHtml(simpleCodeWithLines({
        "class StoneEnemy : public Creature {",
        "public:",
        "    void onHit(Creature& attacker) override {",
        "        attacker.takeDamage(3);",
        "    }",
        "};"
    }));

    Creature* bomb = makeActor("bomb", "Bomb Enemy", "BombEnemy", 1, 0, false, false, "Puzzle Object", this);
    QString bombCode;
    addLine(bombCode, "class BombEnemy : public Creature {");
    addLine(bombCode, "public:");
    addLine(bombCode, "    void onHit(Creature& attacker) override {");
    addLine(bombCode, "        explodeAround();");
    addLine(bombCode, "    }");
    addLine(bombCode, "};");
    addLine(bombCode);
    addRawLine(bombCode, linkCmd("player.attack(bomb);", "player.attack(bomb);") );
    bomb->setCustomClassCodeHtml(codeBlock(bombCode));

    registerCreature("player", player);
    registerCreature("boss", boss);
    registerCreature("stone", stone);
    registerCreature("bomb", bomb);

    m_maxSteps = 3;
    m_currentLevel = 6;
    m_intro =
        "关卡 6：虚函数与动态绑定。\n"
        "Player::attack 的参数静态类型是 Creature&，但 onHit 是 virtual。\n"
        "同样的 player.attack(x)，会根据 x 的真实类型调用不同 override。";

    emit levelChanged();
}

void GameLevel::loadLevel7() {
    // 占位关（建设中）：本关暂时空着。放一个训练假人，玩家随手清掉即可继续，
    // 以免卡住关卡进度。后续可在此处填入新的知识点关卡。
    resetCreatures();
<<<<<<< Updated upstream

    Player* player = new Player(this);
    player->setCustomClassCodeHtml(level7PlayerCode());

    Creature* boss = makeActor("boss", "Vampire Boss", "VampireBoss", 18, 6, true, false, "Enemy", this);
    boss->setCustomClassCodeHtml(simpleCodeWithLines({
        "class VampireBoss : public Enemy {",
        "public:",
        "    void afterTurn() {",
        "        if (!silenced) heal(8);",
        "    }",
        "};"
    }));

    Creature* slot1 = makeActor("slot1", "Spell Slot 1", "FireSpell", 1, 0, false, false, "Spell*", this);
    slot1->setCustomClassCodeHtml(simpleCodeWithLines({
        "class FireSpell : public Spell {",
        "public:",
        "    void cast(Creature& target) override {",
        "        target.takeDamage(6);",
        "    }",
        "};"
    }));

    Creature* slot2 = makeActor("slot2", "Spell Slot 2", "SilenceSpell", 1, 0, false, false, "Spell*", this);
    slot2->setCustomClassCodeHtml(simpleCodeWithLines({
        "class SilenceSpell : public Spell {",
        "public:",
        "    void cast(Creature& target) override {",
        "        target.disableHeal();",
        "    }",
        "};"
    }));

    Creature* slot3 = makeActor("slot3", "Spell Slot 3", "MirrorSpell", 1, 0, false, false, "Spell*", this);
    slot3->setCustomClassCodeHtml(simpleCodeWithLines({
        "class MirrorSpell : public Spell {",
        "public:",
        "    void cast(Creature& target) override {",
        "        target.takeDamage(target.atk());",
        "    }",
        "};"
    }));

    registerCreature("player", player);
    registerCreature("boss", boss);
    registerCreature("slot1", slot1);
    registerCreature("slot2", slot2);
    registerCreature("slot3", slot3);

    m_maxSteps = 4;
    m_currentLevel = 7;
    m_intro =
        "关卡 7：多态与纯虚接口。\n"
        "三个槽位的静态类型都可以看作 Spell*，但真实对象不同。\n"
        "VampireBoss 会回血，先找出能封印回血的 override。";

=======
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
>>>>>>> Stashed changes
    emit levelChanged();
}

void GameLevel::loadLevel8() {
    // 友元内鬼：Vault 的弱点 selfDestruct() 是 private，外部不能调用。
    // 但 Vault 在类体里声明了 friend class Saboteur，于是只有 Saboteur 能合法触发它。
    // Rogue 看起来也像帮手，却不是 friend，无法访问 private 成员。
    resetCreatures();
<<<<<<< Updated upstream

    Player* player = new Player(this);
    player->setCustomClassCodeHtml(level8PlayerCode());

    Creature* enemy = makeActor("enemy", "Template Target", "TemplateEnemy", 20, 0, true, false, "Enemy", this);
    enemy->setCustomClassCodeHtml(simpleCodeWithLines({
        "class TemplateEnemy : public Enemy {",
        "public:",
        "    // 普通攻击无效，必须用拥有 apply() 的道具",
        "};"
    }));

    Creature* healPotion = makeActor("healPotion", "Heal Potion", "HealPotion", 1, 0, false, false, "Item", this);
    healPotion->setCustomClassCodeHtml(simpleCodeWithLines({
        "class HealPotion {",
        "public:",
        "    void apply(Creature& target) {",
        "        target.heal(5);",
        "    }",
        "};"
    }));

    Creature* poisonBomb = makeActor("poisonBomb", "Poison Bomb", "PoisonBomb", 1, 0, false, false, "Item", this);
    poisonBomb->setCustomClassCodeHtml(simpleCodeWithLines({
        "class PoisonBomb {",
        "public:",
        "    void apply(Creature& target) {",
        "        target.takeDamage(20);",
        "    }",
        "};"
    }));

    registerCreature("player", player);
    registerCreature("enemy", enemy);
    registerCreature("healPotion", healPotion);
    registerCreature("poisonBomb", poisonBomb);

    m_maxSteps = 3;
    m_currentLevel = 8;
    m_intro =
        "关卡 8：函数模板与模板参数推导。\n"
        "use(Item& item, Creature& target) 会根据第一个实参推导 Item。\n"
        "同一个模板函数，传入不同类型，调用的 apply() 也不同。";

=======
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
>>>>>>> Stashed changes
    emit levelChanged();
}

void GameLevel::loadLevel9() {
    // 共享所有权：SoulGem 由若干 Cultist 各自持有一份 shared_ptr（owner），
    // Lich 只用 weak_ptr 观察（不计入 use_count）。命匣的「HP」就是它的 use_count，
    // 只有当所有 owner 都消失（use_count → 0）命匣才析构，Lich 的 weak_ptr 失效、才会破防。
    resetCreatures();
<<<<<<< Updated upstream

    Player* player = new Player(this);
    player->setCustomClassCodeHtml(level9PlayerCode());

    Creature* enemy = makeActor("enemy", "Locked Enemy", "LockedEnemy", 20, 0, true, false, "Enemy", this);
    enemy->setCustomClassCodeHtml(simpleCodeWithLines({
        "class LockedEnemy : public Enemy {",
        "public:",
        "    // 被 TemplateDoor 保护，门没开时无法使用炸弹",
        "};"
    }));

    Creature* door = makeActor("door", "Template Door", "TemplateDoor", 1, 0, false, false, "Door", this);
    door->setCustomClassCodeHtml(simpleCodeWithLines({
        "class TemplateDoor {",
        "public:",
        "    void open(Key key);",
        "};"
    }));

    Creature* box1 = makeActor("box1", "Box<HealPotion>", "Box<HealPotion>", 1, 0, false, false, "Box", this);
    box1->setCustomClassCodeHtml(simpleCodeWithLines({
        "Box<HealPotion> box1;",
        "// take() 返回 HealPotion"
    }));

    Creature* box2 = makeActor("box2", "Box<PoisonBomb>", "Box<PoisonBomb>", 1, 0, false, false, "Box", this);
    box2->setCustomClassCodeHtml(simpleCodeWithLines({
        "Box<PoisonBomb> box2;",
        "// take() 返回 PoisonBomb"
    }));

    Creature* box3 = makeActor("box3", "Box<Key>", "Box<Key>", 1, 0, false, false, "Box", this);
    box3->setCustomClassCodeHtml(simpleCodeWithLines({
        "Box<Key> box3;",
        "// take() 返回 Key"
    }));

    registerCreature("player", player);
    registerCreature("enemy", enemy);
    registerCreature("door", door);
    registerCreature("box1", box1);
    registerCreature("box2", box2);
    registerCreature("box3", box3);
=======
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
>>>>>>> Stashed changes

    m_maxSteps = 7;
    m_currentLevel = 9;
<<<<<<< Updated upstream
    m_intro =
        "关卡 9：类模板实例化。\n"
        "Box<HealPotion>、Box<PoisonBomb>、Box<Key> 来自同一个类模板，但它们是不同的具体类型。\n"
        "先拿 Key 开门，再拿 PoisonBomb。";

=======
    m_levelName = GameLevel::levelName(9);
    m_intro = m_levelName;
    m_allowEnemyControl = false;
    setCounter("level9_refcount", 3);
>>>>>>> Stashed changes
    emit levelChanged();
}

void GameLevel::loadLevel10() {
    // 菱形继承：Paladin : public Healer, public Warrior，二者各自继承 Unit 且未用虚继承。
    // 于是 Paladin 中有两个独立的 Unit 子对象（两条 hp：Healer::hp 与 Warrior::hp）。
    // 直接 paladin.expose(...) 有二义性；必须用作用域限定 paladin.Healer::expose / paladin.Warrior::expose
    // 分别清零两条 hp。Healer 半身每回合治疗 Warrior 子对象，所以要先清掉 Healer。
    resetCreatures();
<<<<<<< Updated upstream

    Player* player = new Player(this);
    player->setCustomClassCodeHtml(level10PlayerCode());

    Creature* boss = makeActor("boss", "Undefined Boss", "UndefinedBoss", 40, 8, true, false, "Enemy", this);
    boss->setCustomClassCodeHtml(simpleCodeWithLines({
        "class UndefinedBoss : public Enemy {",
        "public:",
        "    bool typeShield = true;",
        "    bool silenced = false;",
        "};"
    }));

    Creature* mimic = makeActor("mimic", "Final Mimic", "Mimic", 1, 0, false, false, "Puzzle Object", this);
    mimic->setCustomClassCodeHtml(level10MimicCode());

    Creature* ruby = makeActor("ruby", "Final Ruby", "Gem", 1, 0, false, false, "Item", this);
    ruby->setCustomClassCodeHtml(rubyCode());

    Creature* slot = makeActor("slot", "Spell Slot", "SilenceSpell", 1, 0, false, false, "Spell*", this);
    slot->setCustomClassCodeHtml(simpleCodeWithLines({
        "Spell* slot = new SilenceSpell();",
        "slot->cast(boss); // virtual dispatch 到 SilenceSpell::cast"
    }));

    Creature* bomb = makeActor("bomb", "Virtual Bomb", "BombEnemy", 1, 0, false, false, "Puzzle Object", this);
    bomb->setCustomClassCodeHtml(simpleCodeWithLines({
        "class BombEnemy : public Creature {",
        "public:",
        "    void onHit(Creature& attacker) override {",
        "        explodeAround();",
        "    }",
        "};"
    }));

    Creature* weakDragon = makeActor("weakDragon", "Weak Dragon", "DragonPart", 1, 0, false, false, "DragonPart", this);
    weakDragon->setCustomClassCodeHtml(simpleCodeWithLines({
        "class DragonPart : public Creature {",
        "public:",
        "    int hp = 1;",
        "};",
        "",
        "template <class T>",
        "void swapHp(T& a, T& b);",
        "// weakDragon 和 boss 都是 DragonPart 系列，类型可匹配"
    }));

    registerCreature("player", player);
    registerCreature("boss", boss);
    registerCreature("mimic", mimic);
    registerCreature("ruby", ruby);
    registerCreature("slot", slot);
    registerCreature("bomb", bomb);
    registerCreature("weakDragon", weakDragon);

    m_maxSteps = 8;
    m_currentLevel = 10;
    m_intro =
        "关卡 10：Compiler Dragon 终局综合战。\n"
        "你需要连续使用继承、运算符重载、多态、虚函数、函数模板和析构函数。\n"
        "错误命令通常也会告诉你为什么错。";

=======
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
>>>>>>> Stashed changes
    emit levelChanged();
}

Creature* GameLevel::creature(const QString& id) const {
    return m_creatures.value(id, nullptr);
}

QList<Creature*> GameLevel::creatures() const {
    return m_creatures.values();
}

int GameLevel::usedSteps() const { return m_usedSteps; }
int GameLevel::maxSteps() const { return m_maxSteps; }
<<<<<<< Updated upstream
int GameLevel::remainingSteps() const { return m_maxSteps - m_usedSteps; }
=======
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
>>>>>>> Stashed changes

void GameLevel::consumeStep() {
    ++m_usedSteps;
    emit levelChanged();
}

int GameLevel::currentLevel() const { return m_currentLevel; }
bool GameLevel::allowEnemyControl() const { return m_allowEnemyControl; }
QString GameLevel::intro() const { return m_intro; }

void GameLevel::setFlag(const QString& key, bool value) {
    m_flags.insert(key, value);
    emit levelChanged();
}

bool GameLevel::flag(const QString& key) const {
    return m_flags.value(key, false);
}

void GameLevel::clearFlags() {
    m_flags.clear();
    emit levelChanged();
}

bool GameLevel::isWin() const {
    for (Creature* c : m_creatures) {
<<<<<<< Updated upstream
        if (c && c->isEnemy() && c->countsForWin() && c->isAlive()) {
            return false;
        }
=======
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
>>>>>>> Stashed changes
    }
    return true;
}

bool GameLevel::isLose() const {
    Creature* p = creature("player");
    if (!p || !p->isAlive()) return true;
    return m_usedSteps >= m_maxSteps && !isWin();
}
