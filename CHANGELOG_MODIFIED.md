# 本次修改说明

- 替换并新增了第 4、5、7、8、9、10 关的角色/道具插图资源，使其更接近项目原有的奇幻 RPG 贴图风格。
- 第 7 关新增 `enemy_dummy.png` 并在 `BattleSceneView::spriteFor` 中绑定训练假人贴图。
- 第 4 关 `place(c)` 已在源码面板和通关手册中补充说明：它会把当前 `c` 对象放回 `king` 的位置，取代原来的 `king`。
- `throne.seat(king);` 现在会在运行逻辑中实际创建一个普通 `Creature` 版的 `king` 来替换原来的 `ImmortalKing`，与教学说明保持一致。
