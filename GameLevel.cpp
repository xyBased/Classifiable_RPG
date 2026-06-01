#include "GameLevel.h"

#include <QtGlobal>

GameLevel::GameLevel(QObject* parent) : QObject(parent) {}
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
    m_usedSteps = 0;
    m_allowEnemyControl = false;
    m_intro.clear();
}

void GameLevel::registerCreature(const QString& id, Creature* c) {
    m_creatures.insert(id, c);
    connect(c, &Creature::changed, this, &GameLevel::levelChanged);
}

void GameLevel::loadLevel1() {
    resetCreatures();
    Player* player = new Player(this);
    Enemy* enemy = new Enemy(this);

    player->setExtraCodeHtml(htmlBlock("可见实现",
        "void Player::attack(Creature& enemy) {\n"
        "    enemy.takeDamage(atk);\n"
        "}\n\n"
        "bool GameLevel::isWin() const {\n"
        "    return allEnemiesDead();\n"
        "}"));

    enemy->setIntent(Creature::IntentAttack, enemy->atk());
    registerCreature("player", player);
    registerCreature("enemy", enemy);
    m_maxSteps = 5;
    m_currentLevel = 1;
    m_intro = "关卡 1";
    emit levelChanged();
}

void GameLevel::loadLevel2() {
    resetCreatures();
    Player* player = new Player(this);
    Enemy* e1 = new Enemy("enemy1", "Goblin", "Goblin", 3, 7, this);
    Enemy* e2 = new Enemy("enemy2", "Slime King", "SlimeKing", 7, 20, this);

    e1->clearMethods();
    e2->clearMethods();
    e1->addMethod({ "void attack(Creature& target);", "enemy1.attack(enemy2);" });
    e2->addMethod({ "void attack(Creature& target);", "enemy2.attack(enemy1);" });

    QString publicCode =
        "class Enemy : public Creature {\n"
        "public:\n"
        "    void attack(Creature& target);\n"
        "};";
    e1->setExtraCodeHtml(htmlBlock("Enemy 接口", publicCode));
    e2->setExtraCodeHtml(htmlBlock("Enemy 接口", publicCode));

    e1->setIntent(Creature::IntentAttack, e1->atk());
    e2->setIntent(Creature::IntentAttack, e2->atk());

    registerCreature("player", player);
    registerCreature("enemy1", e1);
    registerCreature("enemy2", e2);
    m_maxSteps = 8;
    m_currentLevel = 2;
    m_allowEnemyControl = true;
    emit levelChanged();
}

void GameLevel::loadLevel3() {
    resetCreatures();
    Player* p = new Player(this);
    Enemy* e = new Enemy("enemy", "Destructor Wisp", "DestructorWisp", 30, 4, this);

    p->addMethod({ "void plantBomb(Creature& target);", "player.plantBomb(enemy);" });
    p->setExtraCodeHtml(htmlBlock("局部对象",
        "class ScopedBomb {\n"
        "    Creature& target;\n"
        "public:\n"
        "    ScopedBomb(Creature& t) : target(t) {}\n"
        "    ~ScopedBomb() { target.takeDamage(999); }\n"
        "};\n\n"
        "void Player::plantBomb(Creature& target) {\n"
        "    ScopedBomb bomb(target);\n"
        "}"));

    e->setIntent(Creature::IntentAttack, e->atk());
    registerCreature("player", p);
    registerCreature("enemy", e);
    m_maxSteps = 4;
    m_currentLevel = 3;
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
    goblin->addMethod({ "void attack(Creature& target);", "goblin.attack(boss);" });
    mimic->addMethod({ "void attack(Creature& target);", "mimic.attack(boss);" });

    goblin->setExtraCodeHtml(htmlBlock("public 继承",
        "class Goblin : public Enemy {\n"
        "};"));
    mimic->setExtraCodeHtml(htmlBlock("private 继承",
        "class Mimic : private Enemy {\n"
        "};"));

    goblin->setIntent(Creature::IntentAttack, goblin->atk());
    mimic->setIntent(Creature::IntentDefend, 0);
    boss->setIntent(Creature::IntentAttack, boss->atk());

    registerCreature("player", p);
    registerCreature("goblin", goblin);
    registerCreature("mimic", mimic);
    registerCreature("boss", boss);

    m_maxSteps = 7;
    m_currentLevel = 4;
    m_allowEnemyControl = true;
    emit levelChanged();
}

void GameLevel::loadLevel5() {
    resetCreatures();
    Player* p = new Player(this);
    Enemy* e = new Enemy("enemy", "Armor Boss", "ArmorBoss", 12, 5, this);

    e->setExtraCodeHtml(htmlBlock("护甲判定",
        "void ArmorBoss::onHit(int damage) {\n"
        "    if (damage < 6) return;\n"
        "    hp -= damage;\n"
        "}"));
    p->setExtraCodeHtml(htmlBlock("攻击差异",
        "void Player::attack(Creature& enemy) { enemy.takeDamage(atk); }\n"
        "void Player::powerAttack(Creature& enemy) { enemy.takeDamage(atk * 2); }"));

    e->setIntent(Creature::IntentAttack, e->atk());
    registerCreature("player", p);
    registerCreature("enemy", e);

    m_maxSteps = 5;
    m_currentLevel = 5;
    emit levelChanged();
}

void GameLevel::loadLevel6() {
    resetCreatures();
    Player* p = new Player(this);
    Enemy* boss = new Enemy("boss", "Shielded Boss", "ShieldedBoss", 18, 4, this);
    Enemy* bomb = new Enemy("bomb", "Volatile Bomb", "VolatileBomb", 1, 0, this);
    Enemy* stone = new Enemy("stone", "Stone Enemy", "StoneEnemy", 7, 0, this);

    boss->setExtraCodeHtml(htmlBlock("virtual 分发",
        "class Creature {\n"
        "public:\n"
        "    virtual void onHit(int damage);\n"
        "};\n\n"
        "class ShieldedBoss : public Creature {\n"
        "    void onHit(int damage) override;\n"
        "};\n\n"
        "class VolatileBomb : public Creature {\n"
        "    void onHit(int damage) override;\n"
        "};"));

    bomb->setIntent(Creature::IntentUnknown, 0);
    stone->setIntent(Creature::IntentDefend, 0);
    boss->setIntent(Creature::IntentAttack, boss->atk());

    registerCreature("player", p);
    registerCreature("boss", boss);
    registerCreature("bomb", bomb);
    registerCreature("stone", stone);

    m_maxSteps = 6;
    m_currentLevel = 6;
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

    healer->setExtraCodeHtml(htmlBlock("Intent",
        "enum Intent { Attack, Defend, Heal, Buff, Unknown };\n"
        "Intent Enemy::previewIntent() const;"));

    registerCreature("player", p);
    registerCreature("healer", healer);
    registerCreature("brute", brute);
    registerCreature("captain", captain);

    m_maxSteps = 8;
    m_currentLevel = 8;
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
    m_allowEnemyControl = true;
    emit levelChanged();
}

Creature* GameLevel::creature(const QString& id) const { return m_creatures.value(id, nullptr); }
QList<Creature*> GameLevel::creatures() const { return m_creatures.values(); }

int GameLevel::usedSteps() const { return m_usedSteps; }
int GameLevel::maxSteps() const { return m_maxSteps; }
int GameLevel::remainingSteps() const { return qMax(0, m_maxSteps - m_usedSteps); }
int GameLevel::currentLevel() const { return m_currentLevel; }
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
