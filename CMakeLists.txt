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
find_package(KF6I18n REQUIRED)
find_package(KF6IconThemes REQUIRED)

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

# Install icons (for when app is installed)
set(KDE_INSTALL_ICONDIR "${CMAKE_INSTALL_DATADIR}/icons")
install(
    DIRECTORY icons/hicolor
    DESTINATION "${KDE_INSTALL_ICONDIR}"
)

# Make icons available in build directory (for debug mode)
file(COPY icons/hicolor DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Install desktop file (which will reference your icon)
set(KDE_INSTALL_APPDIR "${CMAKE_INSTALL_DATADIR}/applications")
configure_file(kracker.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kracker.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kracker.desktop DESTINATION ${KDE_INSTALL_APPDIR})

# Install app
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

# Packaging configuration
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION "Graphical interface for Hashcat & John The Ripper")
set(CPACK_PACKAGE_CONTACT "Mohammadreza Hajianpour <hajianpour.mr@gmail.com>")
set(CPACK_PACKAGE_VENDOR "Mohammadreza Hajianpour")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "kracker")

# DEB specific settings
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Mohammadreza Hajianpour <hajianpour.mr@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6 (>= 6.2), libqt6gui6 (>= 6.2), libqt6widgets6 (>= 6.2), kcoreaddons, ki18n, kwidgetsaddons")

# RPM specific settings
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase >= 6.2, kf6-kcoreaddons, kf6-ki18n, kf6-kwidgetsaddons")
set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")

include(CPack)