# Classifiable_RPG final fixed package

把本目录内容覆盖到仓库根目录即可。

本版已修复：
- `QGraphicsScene::addRoundedRect` 不存在导致的构建错误。
- 补齐若干头文件 include。
- 去掉 Qt Multimedia 强依赖，避免部分 Qt 环境找不到 Multimedia 模块。
- Qt5/Qt6 的 CMake 源文件列表保持一致。

主要完成项：
- 新增开始界面：新的旅程 / 继续你的旅程 / 退出。
- 第 5 关及之后自动保存继续进度。
- 新增背景图、玩家、怪物、Boss 贴图。
- 新增攻击/受击/治疗动画、简单特效和音效。
- 新增敌人意图 UI：攻击、治疗、防御、强化、未知。
- 重做右侧状态面板，只保留 HP、ATK、阵营，并把源码区扩展为类定义 + 关键函数实现。
- 移除每关开始时系统日志里的直接提示。
- Player::attack、Player::powerAttack、Player::heal 在所有关卡永久保留。
