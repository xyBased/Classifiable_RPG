#include "GameLevel.h"

#include <initializer_list>
#include <QStringList>
#include <QtAlgorithms>

namespace {

QString compactCommand(QString command) {
    command.remove(' ');
    command.remove('\t');
    command.remove('\n');
    command.remove('\r');
    return command;
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
    resetCreatures();

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

    emit levelChanged();
}

void GameLevel::loadLevel5() {
    resetCreatures();

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
    resetCreatures();

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

    emit levelChanged();
}

void GameLevel::loadLevel8() {
    resetCreatures();

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

    emit levelChanged();
}

void GameLevel::loadLevel9() {
    resetCreatures();

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

    m_maxSteps = 5;
    m_currentLevel = 9;
    m_intro =
        "关卡 9：类模板实例化。\n"
        "Box<HealPotion>、Box<PoisonBomb>、Box<Key> 来自同一个类模板，但它们是不同的具体类型。\n"
        "先拿 Key 开门，再拿 PoisonBomb。";

    emit levelChanged();
}

void GameLevel::loadLevel10() {
    resetCreatures();

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
int GameLevel::remainingSteps() const { return m_maxSteps - m_usedSteps; }

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
        if (c && c->isEnemy() && c->countsForWin() && c->isAlive()) {
            return false;
        }
    }
    return true;
}

bool GameLevel::isLose() const {
    Creature* p = creature("player");
    if (!p || !p->isAlive()) return true;
    return m_usedSteps >= m_maxSteps && !isWin();
}
