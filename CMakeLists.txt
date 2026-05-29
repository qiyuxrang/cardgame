cmake_minimum_required(VERSION 3.1)

set(APP_NAME CardGame)
project(${APP_NAME})
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指向 cocos2d-x 3.17 SDK 根目录 (通过环境变量 COCOS2DX_ROOT 指定，默认为相对路径)
if(DEFINED ENV{COCOS2DX_ROOT})
    set(COCOS2DX_ROOT_PATH "$ENV{COCOS2DX_ROOT}")
else()
    set(COCOS2DX_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cocos2d-x-3.17")
endif()
set(CMAKE_MODULE_PATH ${COCOS2DX_ROOT_PATH}/cmake/Modules/)

include(CocosBuildSet)
if(NOT USE_COCOS_PREBUILT)
    add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos ${ENGINE_BINARY_PATH}/cocos/core)
endif()

set(GAME_SOURCE)
set(GAME_HEADER)

# 资源目录
set(GAME_RES_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
if(APPLE OR WINDOWS)
    cocos_mark_multi_resources(common_res_files RES_TO "Resources" FOLDERS ${GAME_RES_FOLDER})
endif()

# 头文件路径
include_directories(
    Classes
    Classes/Model
    Classes/Controller
    Classes/View
    ${COCOS2DX_ROOT_PATH}/cocos/audio/include/
)

# ---- 源文件清单 ----
list(APPEND GAME_SOURCE
    Classes/AppDelegate.cpp
    Classes/View/GameScene.cpp
    Classes/View/CardSprite.cpp
    Classes/View/TableAreaView.cpp
    Classes/View/HandAreaView.cpp
    Classes/Controller/GameController.cpp
    Classes/Controller/UndoManager.cpp
    Classes/Controller/Actions.cpp
    Classes/Model/CardTypes.cpp
    Classes/Model/GameBoard.cpp
)

list(APPEND GAME_HEADER
    Classes/AppDelegate.h
    Classes/View/GameScene.h
    Classes/View/CardSprite.h
    Classes/View/TableAreaView.h
    Classes/View/HandAreaView.h
    Classes/Controller/GameController.h
    Classes/Controller/UndoManager.h
    Classes/Controller/Actions.h
    Classes/Controller/MatchRule.h
    Classes/Model/CardTypes.h
    Classes/Model/GameBoard.h
    Classes/Model/IAction.h
)

# ---- 平台特定源文件 ----
if(ANDROID)
    set(APP_NAME CardGame)
    list(APPEND GAME_SOURCE proj.android/app/jni/hellocpp/main.cpp)
elseif(LINUX)
    list(APPEND GAME_SOURCE proj.linux/main.cpp)
elseif(WINDOWS)
    list(APPEND GAME_HEADER proj.win32/main.h proj.win32/resource.h)
    list(APPEND GAME_SOURCE proj.win32/main.cpp proj.win32/game.rc ${common_res_files})
elseif(APPLE)
    if(IOS)
        list(APPEND GAME_HEADER proj.ios_mac/ios/AppController.h proj.ios_mac/ios/RootViewController.h)
        set(APP_UI_RES
            proj.ios_mac/ios/LaunchScreen.storyboard
            proj.ios_mac/ios/LaunchScreenBackground.png
            proj.ios_mac/ios/Images.xcassets)
        list(APPEND GAME_SOURCE
            proj.ios_mac/ios/main.m proj.ios_mac/ios/AppController.mm
            proj.ios_mac/ios/RootViewController.mm proj.ios_mac/ios/Prefix.pch
            ${APP_UI_RES})
    elseif(MACOSX)
        set(APP_UI_RES proj.ios_mac/mac/Icon.icns proj.ios_mac/mac/Info.plist)
        list(APPEND GAME_SOURCE proj.ios_mac/mac/main.cpp proj.ios_mac/mac/Prefix.pch ${APP_UI_RES})
    endif()
    list(APPEND GAME_SOURCE ${common_res_files})
endif()

set(APP_SRC ${GAME_SOURCE})

cocos_build_app(${APP_NAME}
    APP_SRC "${APP_SRC}"
    DEPEND_COMMON_LIBS "cocos2d"
    DEPEND_ANDROID_LIBS "cocos2d_android"
)

if(APPLE)
    set_target_properties(${APP_NAME} PROPERTIES RESOURCE "${APP_UI_RES}")
    if(MACOSX)
        set_target_properties(${APP_NAME} PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/mac/Info.plist")
    elseif(IOS)
        cocos_pak_xcode(${APP_NAME} INFO_PLIST "iOSBundleInfo.plist.in")
        set_xcode_property(${APP_NAME} ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")
    endif()
elseif(WINDOWS)
    cocos_copy_target_dll(${APP_NAME} COPY_TO ${APP_BIN_DIR})
endif()

if(LINUX OR WINDOWS)
    cocos_copy_target_res(${APP_NAME} COPY_TO ${APP_RES_DIR} FOLDERS ${GAME_RES_FOLDER})
endif()
