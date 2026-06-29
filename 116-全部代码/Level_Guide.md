# 关卡通关手册（Classifiable RPG）

本手册讲清每一关该怎么玩：考察的 C++ 知识点、场景设置、源码里的关键线索、完整通关步骤，以及"为什么想当然的打法行不通"。

> 第 1、2、3、6 关沿用原设计，简要回顾；第 4、5、7、8、9、10 关为重做/新增关卡，详细攻略。

---

## 通用规则（先读这一段）

- **操作方式**：右侧"源码"面板里，凡是**可点击的蓝色函数签名**都是当前可调用的 `public` 接口。点击会把对应指令填进输入框；也可直接手动输入。指令统一为 `对象.方法(参数);`。
- **回合与步数**：指令**成功执行**才消耗 1 步，并触发所有存活敌人按各自"意图"行动一次；**指令失败不消耗步数、也不触发敌人**，可放心试错。
- **胜负**：消灭所有"真正的敌人"即过关（道具型/教学对象不计入）；玩家 HP 归零或步数耗尽且未过关即失败。
- **看类名按钮**：面板上方的类名按钮（如 `Creature`、`Berserker`、`Paladin`）可切换查看**类定义**；理解继承、`virtual`、`static`、`friend` 等常是解题钥匙。

---

## 第 1 关：平A穿插普攻 —— 成员函数调用
最基本的"对象调用 public 成员函数"。场上只有一个很弱的 `bug`。反复 `player.attack(bug);` 即可。熟悉"读签名 → 点函数 → 选目标"的循环。

## 第 2 关：借刀清场 —— 访问权限与"可控对象"
`darkknight` 的 `attack()` 是 `protected`，你无法直接驱使它；但 `goblin.attack(...)` 是 `public`，且敌人可成为另一个敌人的目标。用 `goblin.attack(darkknight);` 借刀杀人，再清理残局。

## 第 3 关：析构爆破 —— 析构函数 / 对象生命周期 / RAII
`player.summonBomb()` 召唤一个全局 `Bomb`：存在期间令 `player.DEF + 2`，**3 个玩家行动回合后自动析构**，析构时对全体敌方造成 10 点 AoE。`player.defend()` 生成等于当前 DEF 的护盾。先布雷、用 `defend()` 扛过爆发，等 Bomb 析构清场。

---

## 第 4 关：对象切片 —— 传值 vs 传引用 / object slicing

**考点**：把多态对象**按值**传递/拷贝会发生"对象切片"，派生部分（含虚函数重写）被丢弃。

### 场景设置
- `player`：HP 14，ATK 3。
- `king`（ImmortalKing）：HP 9，ATK 3，重写 `onHit` 实现**无敌**。
- `throne`：道具对象，提供两个接口。
- 步数上限：5。

### 源码关键线索
`ImmortalKing::onHit` 直接 `return;`，吸收一切伤害。再看 `Throne` 的两个接口——**注意形参有没有 `&`**：

```cpp
// place(c) 是本关的教学引擎函数，不是 C++ 标准库函数。
// 它会把当前参数 c 所代表的对象放回场上 king 的位置，取代原来的 king。
void Throne::seat(Creature c) {     // 按值传参（没有 &）
    // 进入函数前，ImmortalKing king 被拷贝到 Creature c。
    // 这一步已经发生对象切片：c 只剩 Creature 基类部分，
    // king 的派生部分（含 onHit override）被切掉。
    place(c);   // 用这个普通 Creature c 替换场上的 king
}
void Throne::crown(Creature& c) {   // 按引用传参（有 &）
    // c 不是副本，而是原 king 的别名，仍指向真正的 ImmortalKing
    place(c);   // 放回的仍是 ImmortalKing，无敌不变
}

// 教学伪代码：展示 place(c) 在游戏引擎里的替换效果
void place(/* 当前参数对象 */ CreatureLike c) {
    world.remove("king");
    world.add("king", copy_of_current_object(c));
}
```

### 通关步骤（最优 4 步）
1. `throne.seat(king);` —— 按值接收，先得到被切片的普通 `Creature c`；`place(c)` 再把这份 `c` 放回场上，取代原来的 `king`，所以无敌消失。
2. `player.attack(king);`
3. `player.attack(king);`
4. `player.attack(king);`

### 为什么想当然的打法行不通
- **直接硬刚**：`player.attack(king)` 触发 `ImmortalKing::onHit`，伤害被吸收为 0，仍消耗步数并挨打。
- **用 crown 而不是 seat**：`throne.crown(Creature& c)` 按引用传参，多态保留，king 依旧无敌——解题唯一钥匙是那个没有 `&` 的按值形参 `seat`。

### 学到的知识点
多态对象一旦"按值"拷贝进基类类型，派生部分会被切掉、虚函数重写失效（object slicing）。这是"多态对象要按引用或指针传递"的根本原因。

---

## 第 5 关：静态狂暴（升级版）—— 静态数据成员 / 静态成员函数

**考点**：`static` 数据成员（属于类、被所有实例共享）；`static` 成员函数（用 `类名::函数()` 调用）。

### 核心机制（本次重做）
- **你每攻击一次**：静态成员 `sharedRage++`；随后**所有存活 Berserker** 执行 `atk += sharedRage`。
- 也就是说：**越打，怪物们的攻击力越高**，而且这是"全类共享"地累加——伤害像滚雪球。
- `Berserker::calm()`（静态成员函数）把 `sharedRage` 清零，并将**所有** Berserker 的攻击力恢复为原值（基础 ATK 1）。

### 场景设置
- `player`：HP 14，ATK 3。
- `berserker1` / `berserker2`：HP 6，基础 ATK 1（每次你攻击后会按上面的规则升高）。
- 步数上限：7。

### 源码关键线索
```cpp
int Berserker::sharedRage = 0;   // 静态数据成员：全类只有一份

// 玩家每攻击一次，引擎对所有存活 Berserker 调用 enrage()：
void Berserker::enrage() {
    sharedRage++;        // 共享计数 +1（全类同步）
    atk += sharedRage;   // 攻击力叠加当前 sharedRage —— 越打越疼
}

static void Berserker::calm() {   // 静态成员函数：Berserker::calm()
    sharedRage = 0;               // 清零共享计数
    // 同时把所有 Berserker 的 atk 恢复为原值
}
```

> 注意敌人头顶的"攻击 N"意图会随你的攻击实时变大——这就是 `sharedRage` 在全类同步生效的可视化。

### 通关步骤（一种最优 5 步线）
1. `player.attack(berserker1);` —— b1 6→3。`sharedRage→1`，两只 atk 都变 2。回合末挨 2+2=4。
2. `player.attack(berserker1);` —— b1 3→0 阵亡。`sharedRage→2`，存活的 b2 atk 4。挨 4。
3. `Berserker::calm();` —— `sharedRage→0`，b2 atk 复位为 1。挨 1。
4. `player.attack(berserker2);` —— b2 6→3。`sharedRage→1`，b2 atk 2。挨 2。
5. `player.attack(berserker2);` —— b2 3→0 阵亡。通关（剩约 3 HP）。

要点：**在攻击力滚到危险值之前，用 `calm()` 把全类清零**，再继续输出。

### 为什么想当然的打法行不通
- **一路猛攻不冷静**：`sharedRage` 越滚越高，怪物攻击力指数般上升。实测不冷静会在第 3 次攻击的反击中被打死。
- **指望杀掉一只来降低难度**：杀死一只**不会**重置 `sharedRage`（它属于类、不属于实例），存活那只反而带着累加后的高攻继续猛揍你。
- 唯一稳妥的"刹车"是 `Berserker::calm()`——静态成员函数，一次调用把全类的攻击力清回原值。

### 学到的知识点
`static` 数据成员在所有实例间只有一份，是"类级别的共享状态"；`static` 成员函数不依附具体对象，用 `ClassName::func()` 调用，常用来操作这种共享状态。本关让你亲手体会"共享"既带来联动增益，也需要类级别的手段来统一管理。

---

## 第 6 关：虚函数分发 —— 多态 / virtual onHit
`player.attack(...)` 对不同对象触发各自重写的 `onHit`：`boss`（护盾吸收普通攻击）、`stone`（免疫并反弹 3 点）、`bomb`（受击引爆，把伤害传导给 boss）。直接打 boss 没用；攻击 `bomb` 让爆炸带走 boss。`stone` 是教学反伤墙，无需击杀。

---

## 第 7 关：真身显形 —— dynamic_cast 向下转型 / RTTI

### 场景设置
- `player`：HP 14，ATK 3。
- `mimic`（拟态魔，类 `Mimic : public Creature`）：HP 8，ATK 3，每回合攻击你 3 点。它的真实类型是 `Mimic`，但场上以 `Creature&` 基类视图交给你。伪装状态下重写的 `onHit` 吸收一切伤害，直接攻击无效。
- `appraiser`（鉴定师，道具）：公开方法 `void reveal(Creature& c);`，内部用 `dynamic_cast<Mimic*>` 向下转型。
- `decoy`（诱饵，类 `DecoyChest : public Creature`）：一个伪装成 Mimic 的箱子，真实类型不是 Mimic，用来演示 `dynamic_cast` 失配时返回 `nullptr`。

### 源码关键线索
- `Mimic::revealWeakness()` 是 `Mimic` 派生类独有的成员，基类 `Creature` 里没有它。你拿到的是 `Creature&`，编译期看不到这个方法，所以不能写 `mimic.revealWeakness()`。
- `Appraiser::reveal(Creature& c)` 的实现：

  ```
  void Appraiser::reveal(Creature& c) {
      if (Mimic* m = dynamic_cast<Mimic*>(&c)) {
          m->revealWeakness();
      }
  }
  ```

  `dynamic_cast<Mimic*>(&c)` 在运行期检查 `c` 的真实类型：是 `Mimic` 就返回有效指针，进入 `if` 调用派生类专属的 `revealWeakness()`；不是（如 `decoy`）就返回 `nullptr`，安全跳过。

### 通关步骤（最优 4 步）
1. `appraiser.reveal(mimic);` —— `dynamic_cast` 命中真身，调用 `revealWeakness()` 剥下伪装。
2. `player.attack(mimic);`
3. `player.attack(mimic);`
4. `player.attack(mimic);` —— 8 HP / 每次 3 点，三刀击杀。

（`reveal` 这一步及之后每个回合 mimic 会反击 3 点；player 14 HP 足以撑到击杀，maxSteps 为 6 有余量。）

### 为什么想当然的打法行不通
- 直接 `player.attack(mimic);`：伪装状态下 `onHit` 吸收全部伤害，毫无效果。
- 想直接 `mimic.revealWeakness();`：基类 `Creature&` 视图里根本看不到这个派生类成员，调用不到。
- 对 `decoy` 用 `appraiser.reveal(decoy);`：`dynamic_cast<Mimic*>` 返回 `nullptr`（真实类型是 `DecoyChest`，不是 `Mimic`），安全失败——这正体现了 `dynamic_cast` 相比 `static_cast` 的价值：运行期类型检查，失配返回空指针而不是未定义行为。

### 学到的知识点
- **`dynamic_cast` 向下转型（downcast）**：把基类指针/引用安全地转成派生类，配合多态（虚函数）使用。
- **RTTI（运行时类型识别）**：`dynamic_cast` 在运行期检查对象真实类型，失配时指针版返回 `nullptr`。
- 与第 4 关（对象切片）互补：第 4 关讲「值拷贝会切掉派生部分」，本关讲「基类句柄看不到派生接口时，如何安全地把它取回来」。

---

## 第 8 关：友元内鬼 —— friend 类访问私有成员

**考点**：`friend` 授予的私有访问权限；封装边界。

### 场景设置
- `player`：HP 12，ATK 3。
- `vault`（Vault Boss）：常规攻击无效，弱点 `selfDestruct()` 是 `private`。
- `saboteur`（内鬼）与 `rogue`（普通盗贼）：两个看似都像帮手的道具对象。
- 步数上限：5。

### 源码关键线索
```cpp
class Vault : public Creature {
private:
    friend class Saboteur;     // ★ 只有 Saboteur 是金库的朋友
    void selfDestruct();       // private：外部不可调用
};

void Saboteur::betray(Vault& v) { v.selfDestruct(); }   // 合法：friend
void Rogue::pick(Vault& v)      { /* v.selfDestruct(); 编译错误：非 friend */ }
```

### 通关步骤（最优 1 步）
1. `saboteur.betray(vault);` —— Saboteur 是 Vault 声明的 friend，`betray()` 合法调用 `vault.selfDestruct()`，金库自毁，立即过关。

### 为什么想当然的打法行不通
- `vault.selfDestruct();` —— `private`，外部不能调用。
- `rogue.pick(vault);` —— Rogue 不是 friend，无法访问私有成员。
- `player.attack(vault);` —— 外壳无敌，常规攻击无效，且白费步数挨 4 点。

判据只有一个：Vault 类体里那行 `friend class Saboteur;`。

### 学到的知识点
`friend` 是由类**主动授予**的访问特权：外部拿不到的 `private` 成员，被声明为 friend 的类可以合法访问。

---

## 第 9 关：共享所有权 —— shared_ptr / 引用计数 / weak_ptr

**考点**：`std::shared_ptr` 的共享所有权与引用计数；`weak_ptr` 只观察不延寿。

### 场景设置
- `player`：HP 16，ATK 3。
- `lich`（Lich Boss）：HP 9，ATK 2，命匣在时**免疫一切伤害**。
- `gem`（Soul Gem）：**它的 HP 就是 `use_count`**，初始 3。
- `cultist1/2/3`：三个信徒，**各持有一份 `shared_ptr<SoulGem>`**，每回合各打你 1 点。
- 步数上限：7。

### 源码关键线索
```cpp
class Cultist : public Creature { shared_ptr<SoulGem> gem; };   // 拥有者，use_count += 1

void Lich::onHit(Creature& attacker, int damage) {
    if (!gem.expired()) return;   // 命匣仍有 owner，Lich 免疫
    hp -= damage;                 // use_count 归零、命匣析构后才会受伤
}
```

### 通关步骤（最优 6 步）
1–3. 依次 `player.attack(cultist1/2/3);` —— 每杀一个，释放一份 `shared_ptr`，`use_count` 递减；归零时 `~SoulGem()` 析构，Lich 的 `weak_ptr` 失效。
4–6. `player.attack(lich);` × 3 —— 此时 Lich 可破防，击杀过关。

### 为什么想当然的打法行不通
- **先打 Lich**：`use_count > 0` 时 `onHit` 直接 `return`，免疫。
- **直接打命匣 gem**：被拒绝——它的寿命由引用计数决定，只能通过消灭所有持有者让 `use_count` 归零。

### 学到的知识点
`shared_ptr` 通过引用计数实现共享所有权：资源活到最后一个所有者消失为止；`weak_ptr` 只观察、不计入计数。本关把引用计数可视化成命匣血条。

---

## 第 10 关：菱形继承 —— 多重继承 / 二义性 / 作用域解析

**考点**：多重继承、菱形继承（未用虚继承 → 基类子对象被复制）、用 `Base::成员` 作用域解析。

### 场景设置
- `player`：HP 14，ATK 3。
- `paladin`（Paladin Boss）：`class Paladin : public Healer, public Warrior`。两个半身各有一条 `Unit::hp`（各 6）。
  - **Warrior 半身**：HP 在则每回合攻击你 2 点。
  - **Healer 半身**：HP 在则每回合给 Warrior 子对象回 2 点血。
- 步数上限：7。

### 源码关键线索
```cpp
class Unit { protected: int hp; };                       // 公共基类
class Healer  : public Unit { public: void expose(Player&); };   // 治疗 Warrior 子对象
class Warrior : public Unit { public: void expose(Player&); };   // 攻击 player

class Paladin : public Healer, public Warrior {};   // 未用虚继承：两个独立 Unit 子对象

// paladin.expose(player);            // 二义：两个 expose / 两个 Unit::hp
// paladin.Healer::expose(player);    // 作用域限定，选中 Healer 子对象
// paladin.Warrior::expose(player);   // 选中 Warrior 子对象
```

### 通关步骤（最优 4 步）—— **顺序很重要**
1. `paladin.Healer::expose(player);` —— Healer::hp 6 → 3。
2. `paladin.Healer::expose(player);` —— Healer::hp 3 → 0，**治疗停止**。
3. `paladin.Warrior::expose(player);` —— Warrior::hp 6 → 3。
4. `paladin.Warrior::expose(player);` —— Warrior::hp 3 → 0，Paladin 倒下。

### 为什么想当然的打法行不通
- **不加限定的 `paladin.expose(player)`**：二义性——必须用 `Healer::` / `Warrior::` 作用域限定。
- **直接 `player.attack(paladin)`**：同样触及二义的 `hp`，被拒绝。
- **先打 Warrior 半身**：只要 Healer 半身还活着，每回合把 Warrior 子对象补满，永远清不掉。**必须先清 Healer，再清 Warrior。**

### 学到的知识点
多重继承会引入命名二义性，需用 `Base::member` 作用域解析；菱形继承若不使用虚继承，公共基类会被复制成多个子对象（这里表现为"两条 HP"）。

---

## 难度与知识点速查表

| 关卡 | 名称 | 核心知识点 | 最优步数 |
|---|---|---|---|
| 1 | 平A穿插普攻 | 成员函数调用 | — |
| 2 | 借刀清场 | 访问控制（public/protected） | — |
| 3 | 析构爆破 | 析构函数 / 对象生命周期 / RAII | — |
| 4 | 对象切片 | 传值 vs 传引用 / object slicing | 4 |
| 5 | 静态狂暴 | 静态数据成员 / 静态成员函数（越打越疼） | 5 |
| 6 | 虚函数分发 | 多态 / virtual 重写 | — |
| 7 | 真身显形 | dynamic_cast 向下转型 / RTTI | 4 |
| 8 | 友元内鬼 | friend 访问私有成员 | 1 |
| 9 | 共享所有权 | shared_ptr / 引用计数 / weak_ptr | 6 |
| 10 | 菱形继承 | 多重继承 / 二义性 / 作用域解析 | 4 |

---

## 关于贴图

本版本为狂战士、国王、宝座、金库、内鬼、盗贼、巫妖、命匣、信徒、圣骑士、石巨人等角色新增了专属贴图（统一的扁平描边风格，透明背景）。如果你想换成自己的立绘，只需用同名 PNG 覆盖 `assets/` 目录下对应文件，并保持 `assets/assets.qrc` 中的别名不变即可。
