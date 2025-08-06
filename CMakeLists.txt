cmake_minimum_required(VERSION 3.21)
set(PROJECT_VERSION 0.3)
set(PROJECT_NAME Kracker VERSION ${PROJECT_VERSION} LANGUAGES CXX)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Concurrent)
# Find individual KF6 components
find_package(KF6CoreAddons REQUIRED)
find_package(KF6I18n REQUIRED)
find_package(KF6WidgetsAddons REQUIRED)
find_package(KF6Config REQUIRED)
find_package(KF6Crash REQUIRED)
find_package(KF6DBusAddons REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/MainWindow.hpp
    src/MainWindow.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::Crash
    KF6::DBusAddons
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
