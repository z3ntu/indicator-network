project(systeui)

cmake_minimum_required(VERSION 2.6.0)

find_package(Qt4 REQUIRED)

include(FindPkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(DBUSMENUGLIB dbusmenu-glib-0.4)
add_definitions(-DQT_NO_KEYWORDS)

enable_testing()

add_subdirectory(dbusmodel)
add_subdirectory(qml)
add_subdirectory(examples)
add_subdirectory(tests)
