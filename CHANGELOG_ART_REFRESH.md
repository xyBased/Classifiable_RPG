# Art Refresh Package

已完成第 4–10 关角色/道具插画替换预集成。

## 替换内容
- 第 4 关：Immortal King、Throne
- 第 5 关：Berserker
- 第 6 关：Shielded Boss、Stone Enemy、Volatile Bomb
- 第 7 关：Training Dummy
- 第 8 关：Vault、Saboteur、Rogue
- 第 9 关：Lich、Soul Gem、Cultist
- 第 10 关：Paladin

## 可点击化处理
- 所有新插画都已处理为带透明通道的 PNG。
- `ActorItem` 使用 `QGraphicsPixmapItem::MaskShape`，点击、悬停、选中描边按透明遮罩轮廓响应。
- 第 6 关新增 `enemy_boss_l6.png` 和 `player_bomb_l6.png`，避免影响第 2、3 关原图。
- `assets/new_art_preview_sheet.png` 是本次集成后的资源预览表。


## v5 refined extraction
- 对容易互相串入的几组对象重新手工微调包围框：
  throne / vault / saboteur / rogue / boss / stone / bomb / cultist / gem
- 目标是去掉相邻角色碎片，同时保留目标角色完整肢体与武器。
- 新增 `assets/new_art_preview_sheet_refined_v4.png`。
