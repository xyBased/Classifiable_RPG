# 构建说明（重要）

## 如何打开并编译
1. 用 Qt Creator 打开本目录下的 `CMakeLists.txt`（不要打开任何 `.user` 文件）。
2. 选择一个 Qt 6 + MinGW（或 MSVC）套件（Kit），点击「Configure Project / 配置工程」。
3. 点左下角的「构建并运行（绿色三角）」即可。

本工程**没有附带任何 `.user` 用户配置或 build 缓存**，Qt Creator 会在你的机器上自动生成正确配置，因此不会再出现"源码目录与缓存不匹配"的问题。

## 如果你之前已经构建过、仍报 "source does not match / cache" 错误
这说明你的硬盘上还残留着**旧的 build 目录**（里面的 `CMakeCache.txt` 记着旧路径）。按任一方式清掉即可：

- 方式 A（Qt Creator 内）：菜单「构建 Build」→「清理 Clean」/「重置 / Run CMake」；或在左侧「项目 Projects」页删除已配置的构建目录，重新 Configure。
- 方式 B（手动）：直接删除旧的构建文件夹，例如错误信息里出现过的：
  `D:\Classifiable_RPG-17\Classifiable_RPG-17\build\Desktop_Qt_6_10_0_MinGW_64_bit-Debug`
  以及当前工程旁边任何名为 `build*` 的文件夹，然后回到第 1 步重新 Configure。

CMake 的规则是：一个 build 目录的缓存只能对应**生成它时**的那个源码目录。换了源码位置就必须用新的（或清空的）build 目录。

## 小贴士
- 不要把工程放在中文或含空格的路径下，MinGW 工具链对这类路径有时会出问题。
- 关卡玩法见 `Level_Guide.md`。
- 想替换角色立绘：用同名 PNG 覆盖 `assets/` 下对应文件，保持 `assets/assets.qrc` 中的别名不变即可。
