#include "GameLevel.h"

#include <QtGlobal>

GameLevel::GameLevel(QObject* parent) : QObject(parent) {}

QString GameLevel::levelName(int level) {
    switch (level) {
    case 1: return QStringLiteral("平A穿插普攻");
    case 2: return QStringLiteral("借刀清场");
    case 3: return QStringLiteral("析构爆破");
    case 4: return QStringLiteral("继承权限");
    case 5: return QStringLiteral("护甲判定");
    case 6: return QStringLiteral("虚函数分发");
    case 7: return QStringLiteral("多态法术");
    case 8: return QStringLiteral("意图预判");
    case 9: return QStringLiteral("模板开门");
    case 10: return QStringLiteral("运算符与模板");
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
    resetCreatures();
    Player* p = new Player(this);
    Enemy* goblin = new Enemy("goblin", "Goblin", "Goblin", 8, 12, this);
    Enemy* mimic = new Enemy("mimic", "Mimic", "Mimic", 6, 1, this);
    Enemy* boss = new Enemy("boss", "Inheritance Boss", "InheritanceBoss", 24, 4, this);

    goblin->clearMethods();
    mimic->clearMethods();
    // Goblin 以 public 继承 Creature：基类 public 的 attack 仍可被外部调用。
    // 源码区写成 "class Creature { public: attack };  class Goblin : public Creature {};"，attack 可点击。
    goblin->addMethod({ "void attack(Creature& target);", "goblin.attack(boss);" });
    goblin->setInheritInfo("Creature", "public");
    goblin->setInheritDemo(true);
    goblin->setExtraCodeHtml(QString());

    // Mimic 以 private 继承 Creature：attack 被降级为 private。
    // 源码同样写出可点击的 attack，但运行 mimic.attack(...) 时会报错并解释原因。
    mimic->addMethod({ "void attack(Creature& target);", "mimic.attack(boss);" });
    mimic->setInheritInfo("Creature", "private");
    mimic->setInheritDemo(true);
    mimic->setClassBodyNote("// private 继承：attack 降级为 private，外部调用 mimic.attack(...) 会报错");
    mimic->setExtraCodeHtml(QString());

    goblin->setIntent(Creature::IntentAttack, goblin->atk());
    mimic->setIntent(Creature::IntentDefend, 0);
    boss->setIntent(Creature::IntentAttack, boss->atk());

    registerCreature("player", p);
    registerCreature("goblin", goblin);
    registerCreature("mimic", mimic);
    registerCreature("boss", boss);

    m_maxSteps = 7;
    m_currentLevel = 4;
    m_levelName = GameLevel::levelName(4);
    m_allowEnemyControl = true;
    emit levelChanged();
}

void GameLevel::loadLevel5() {
    resetCreatures();
    Player* p = new Player(this);
    Enemy* e = new Enemy("enemy", "Armor Boss", "ArmorBoss", 12, 5, this);

    // onHit 的函数名写进 ArmorBoss 的类定义中
    e->addClassDeclaration("void onHit(int damage);");
    e->setExtraCodeHtml(htmlBlock("护甲判定",
        "void ArmorBoss::onHit(int damage) {\n"
        "    if (damage < 6) return;\n"
        "    hp -= damage;\n"
        "}"));

    // powerAttack 的函数名写进 Player 类定义，并可被点击调用；
    // attack 的实现已在第一关写出，这里不再重复。
    p->addMethod({ "void powerAttack(Creature& target);", "player.powerAttack(enemy);" });
    p->setExtraCodeHtml(htmlBlock("强化攻击",
        "void Player::powerAttack(Creature& enemy) {\n"
        "    enemy.takeDamage(atk * 2);\n"
        "}"));

    e->setIntent(Creature::IntentAttack, e->atk());
    registerCreature("player", p);
    registerCreature("enemy", e);

    m_maxSteps = 5;
    m_currentLevel = 5;
    m_levelName = GameLevel::levelName(5);
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
    resetCreatures();
    Player* p = new Player(this);
    Creature* slot1 = new Creature("slot1", "FireSpell", "FireSpell", 1, 0, true, this);
    Creature* slot2 = new Creature("slot2", "HealSpell", "HealSpell", 1, 0, true, this);
    Creature* slot3 = new Creature("slot3", "MirrorSpell", "MirrorSpell", 1, 0, true, this);
    Enemy* boss = new Enemy("boss", "Spell Boss", "SpellBoss", 16, 6, this);

    slot1->addMethod({ "void cast(Creature& target);", "slot1.cast(boss);" });
    slot2->addMethod({ "void cast(Creature& target);", "slot2.cast(player);" });
    slot3->addMethod({ "void cast(Creature& target);", "slot3.cast(boss);" });

    QString spellCode =
        "class Spell { public: virtual void cast(Creature& target) = 0; };\n"
        "class FireSpell : public Spell { void cast(Creature& t) override { t.takeDamage(6); } };\n"
        "class HealSpell : public Spell { void cast(Creature& t) override { t.heal(4); } };\n"
        "class MirrorSpell : public Spell { void cast(Creature& t) override { t.takeDamage(t.atk()); } };";
    slot1->setExtraCodeHtml(htmlBlock("Spell 多态", spellCode));
    slot2->setExtraCodeHtml(htmlBlock("Spell 多态", spellCode));
    slot3->setExtraCodeHtml(htmlBlock("Spell 多态", spellCode));

    slot1->setIntent(Creature::IntentUnknown, 0);
    slot2->setIntent(Creature::IntentHeal, 4);
    slot3->setIntent(Creature::IntentUnknown, 0);
    boss->setIntent(Creature::IntentAttack, boss->atk());

    registerCreature("player", p);
    registerCreature("slot1", slot1);
    registerCreature("slot2", slot2);
    registerCreature("slot3", slot3);
    registerCreature("boss", boss);

    m_maxSteps = 7;
    m_currentLevel = 7;
    m_levelName = GameLevel::levelName(7);
    m_allowEnemyControl = true;
    emit levelChanged();
}

void GameLevel::loadLevel8() {
    resetCreatures();
    Player* p = new Player(this);
    Enemy* healer = new Enemy("healer", "Healer", "Healer", 8, 1, this);
    Enemy* brute = new Enemy("brute", "Brute", "Brute", 18, 5, this);
    Enemy* captain = new Enemy("captain", "Captain", "Captain", 10, 3, this);

    healer->setIntent(Creature::IntentHeal, 3);
    brute->setIntent(Creature::IntentAttack, brute->atk());
    captain->setIntent(Creature::IntentBuff, 1);

    // previewIntent 的函数名写进各敌人对应的类定义之中
    healer->addClassDeclaration("Intent previewIntent() const;");
    brute->addClassDeclaration("Intent previewIntent() const;");
    captain->addClassDeclaration("Intent previewIntent() const;");

    healer->setExtraCodeHtml(htmlBlock("Intent",
        "enum Intent { Attack, Defend, Heal, Buff, Unknown };"));

    registerCreature("player", p);
    registerCreature("healer", healer);
    registerCreature("brute", brute);
    registerCreature("captain", captain);

    m_maxSteps = 8;
    m_currentLevel = 8;
    m_levelName = GameLevel::levelName(8);
    emit levelChanged();
}

void GameLevel::loadLevel9() {
    resetCreatures();
    Player* p = new Player(this);
    Enemy* door = new Enemy("door", "Template Door", "TemplateDoor", 1, 0, this);
    Creature* box1 = new Creature("box1", "Box<Potion>", "Box_Potion", 1, 0, true, this);
    Creature* box2 = new Creature("box2", "Box<Coin>", "Box_Coin", 1, 0, true, this);
    Creature* box3 = new Creature("box3", "Box<Key>", "Box_Key", 1, 0, true, this);

    box1->addMethod({ "T take();", "box1.take();" });
    box2->addMethod({ "T take();", "box2.take();" });
    box3->addMethod({ "T take();", "box3.take();" });
    door->clearMethods();
    door->addMethod({ "void open(Key key);", "door.open(key);" });

    QString tmpl =
        "template <class T>\n"
        "class Box { public: T take(); };\n\n"
        "class TemplateDoor {\n"
        "public:\n"
        "    void open(Key key);\n"
        "};";
    box1->setExtraCodeHtml(htmlBlock("模板实参", "Box<Potion> box1;"));
    box2->setExtraCodeHtml(htmlBlock("模板实参", "Box<Coin> box2;"));
    box3->setExtraCodeHtml(htmlBlock("模板实参", "Box<Key> box3;"));
    door->setExtraCodeHtml(htmlBlock("模板门", tmpl));

    door->setIntent(Creature::IntentDefend, 0);
    box1->setIntent(Creature::IntentUnknown, 0);
    box2->setIntent(Creature::IntentUnknown, 0);
    box3->setIntent(Creature::IntentUnknown, 0);

    registerCreature("player", p);
    registerCreature("door", door);
    registerCreature("box1", box1);
    registerCreature("box2", box2);
    registerCreature("box3", box3);

    m_maxSteps = 5;
    m_currentLevel = 9;
    m_levelName = GameLevel::levelName(9);
    m_allowEnemyControl = true;
    emit levelChanged();
}

void GameLevel::loadLevel10() {
    resetCreatures();
    Player* p = new Player(this);
    Enemy* mimic = new Enemy("mimic", "Ruby Mimic", "RubyMimic", 1, 0, this);
    Creature* ruby = new Creature("ruby", "Ruby", "Ruby", 1, 0, true, this);
    Enemy* boss = new Enemy("boss", "Undefined Boss", "UndefinedBoss", 28, 7, this);
    Enemy* weakDragon = new Enemy("weakDragon", "Weak Dragon", "WeakDragon", 3, 0, this);

    mimic->clearMethods();
    mimic->addMethod({ "Ruby open();", "mimic.open();" });
    p->addMethod({ "Player& operator+=(Ruby& ruby);", "player += ruby;" });
    weakDragon->addMethod({ "template<class T> void swapHp(T& a, T& b);", "swapHp(weakDragon, boss);" });

    p->setExtraCodeHtml(htmlBlock("运算符重载",
        "Player& Player::operator+=(Ruby& ruby) {\n"
        "    atk += 7;\n"
        "    return *this;\n"
        "}"));
    weakDragon->setExtraCodeHtml(htmlBlock("函数模板",
        "template <class T>\n"
        "void swapHp(T& a, T& b) {\n"
        "    std::swap(a.hp, b.hp);\n"
        "}"));
    boss->setExtraCodeHtml(htmlBlock("最终 Boss",
        "class UndefinedBoss : public Creature {\n"
        "};"));

    mimic->setIntent(Creature::IntentUnknown, 0);
    ruby->setIntent(Creature::IntentUnknown, 0);
    boss->setIntent(Creature::IntentAttack, boss->atk());
    weakDragon->setIntent(Creature::IntentDefend, 0);

    registerCreature("player", p);
    registerCreature("mimic", mimic);
    registerCreature("ruby", ruby);
    registerCreature("boss", boss);
    registerCreature("weakDragon", weakDragon);

    m_maxSteps = 9;
    m_currentLevel = 10;
    m_levelName = GameLevel::levelName(10);
    m_allowEnemyControl = true;
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
    case 4: return "注意哪些继承方式仍然保留外部可访问接口。";
    case 5: return "普通攻击与强化攻击造成的数值并不相同。";
    case 6: return "不同对象收到同一个攻击时，结果不一定一样。";
    case 7: return "三个 spell 的行为完全不同。";
    case 8: return "先看敌人的意图，再决定你的顺序。";
    case 9: return "门需要对的东西，箱子里不一定每个都对。";
    case 10: return "先拿到 ruby，再想办法把收益最大化。";
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
        if (m_currentLevel == 10 && (c->id() == "ruby" || c->id() == "mimic" || c->id() == "weakDragon")) {
            continue;
        }
        // 第六关：stone 是用来演示 onHit 反弹/免疫的教学敌人，本身无法被击杀，
        // 不计入全灭判定。只要 boss（与连锁引爆的 bomb）被清除即算过关。
        if (m_currentLevel == 6 && c->id() == "stone") {
            continue;
        }
        if (m_currentLevel == 9) {
            Creature* door = creature("door");
            return door && !door->isAlive();
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
