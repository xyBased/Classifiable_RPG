cmake_minimum_required(VERSION 3.16)

project(Classifiable_RPG VERSION 6.9 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)   # 处理所有带 Q_OBJECT 的类
set(CMAKE_AUTOUIC ON)   # 处理 MainWindow.ui
set(CMAKE_AUTORCC ON)   # 处理 assets/assets.qrc

# 本工程只用到 Widgets（含 Core/Gui）。SoundUtil 用的是 QApplication::beep()，
# 不依赖 Qt Multimedia，所以这里无需 Multimedia 组件。
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(Classifiable_RPG
    main.cpp

    ActorItem.cpp        ActorItem.h
    BattleSceneView.cpp  BattleSceneView.h
    CommandPanel.cpp     CommandPanel.h
    CommandParser.cpp    CommandParser.h
    Creature.cpp         Creature.h
    Enemy.cpp            Enemy.h
    GameController.cpp   GameController.h
    GameLevel.cpp        GameLevel.h
    InfoCodePanel.cpp    InfoCodePanel.h
    MainWindow.cpp       MainWindow.h    MainWindow.ui
    Player.cpp           Player.h
    SoundUtil.cpp        SoundUtil.h
    SpriteUtil.cpp       SpriteUtil.h

    assets/assets.qrc
)

target_link_libraries(Classifiable_RPG PRIVATE Qt6::Widgets)

# Windows 下作为 GUI 程序（不弹出控制台黑窗）
if (WIN32)
    set_target_properties(Classifiable_RPG PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
