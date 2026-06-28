# 本次修改说明

## 最新一轮（源码排版重构 + 第 7 关重做）
- **源码展示区注释排版**：第 4 关及以后所有关卡，源码面板由「每行代码后挂行尾注释」改为「先展示整段干净代码（整个类或整个函数），再另起一行用 `//` 给出诠释」。第 1、2、3 关保持原样。
  - 在 `Creature::classCodeHtml`、`Enemy::classCodeHtml`、`Creature::baseClassCodeHtml` 中实现：类体内成员声明剥离行尾注释，统一收集到类 `};` 之后的注释块。
  - `GameLevel.cpp` 中各关 `htmlBlock(...)` 的函数实现字符串逐个重排，函数体保持干净，诠释移到函数之后。
- **第 7 关从占位关重做为真正的知识点关卡**：`真身显形 —— dynamic_cast 向下转型 / RTTI`。
  - 新角色 `mimic`（`Mimic : public Creature`，伪装时 `onHit` 吸伤）、道具 `appraiser`（`reveal` 内部 `dynamic_cast<Mimic*>`）、诱饵 `decoy`（`DecoyChest`，演示 cast 返回 `nullptr`）。
  - 贴图复用现有资源：`mimic`→石巨人 `enemy_stone.png`、`appraiser`→宝珠 `prop_gem.png`、`decoy`→金库 `enemy_vault.png`，未新增 PNG。
  - 配套更新 `CommandParser`（`reveal` / `revealWeakness` / 攻击分支）、`isWin` 排除项、关卡提示与 `Level_Guide.md`。

## 历史修改
- 替换并新增了第 4、5、7、8、9、10 关的角色/道具插图资源，使其更接近项目原有的奇幻 RPG 贴图风格。
- 第 7 关新增 `enemy_dummy.png` 并在 `BattleSceneView::spriteFor` 中绑定训练假人贴图。
- 第 4 关 `place(c)` 已在源码面板和通关手册中补充说明：它会把当前 `c` 对象放回 `king` 的位置，取代原来的 `king`。
- `throne.seat(king);` 现在会在运行逻辑中实际创建一个普通 `Creature` 版的 `king` 来替换原来的 `ImmortalKing`，与教学说明保持一致。
