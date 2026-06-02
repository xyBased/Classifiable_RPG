cmake_minimum_required(VERSION 3.16)

project(RPGByAI VERSION 0.2 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp MainWindow.h MainWindow.ui
    Creature.h Creature.cpp
    Player.h Player.cpp
    Enemy.h Enemy.cpp
    GameLevel.h GameLevel.cpp
    CommandParser.h CommandParser.cpp
    ActorItem.h ActorItem.cpp
    BattleSceneView.h BattleSceneView.cpp
    InfoCodePanel.h InfoCodePanel.cpp
    CommandPanel.h CommandPanel.cpp
    GameController.h GameController.cpp
    SpriteUtil.h SpriteUtil.cpp
    SoundUtil.h SoundUtil.cpp
    assets/assets.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(RPGByAI MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    if(ANDROID)
        add_library(RPGByAI SHARED ${PROJECT_SOURCES})
    else()
        add_executable(RPGByAI ${PROJECT_SOURCES})
    endif()
endif()

target_link_libraries(RPGByAI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.RPGByAI)
endif()

set_target_properties(RPGByAI PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS RPGByAI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(RPGByAI)
endif()
