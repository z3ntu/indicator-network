project(system-settings)

cmake_minimum_required(VERSION 2.6.0)

find_package(Qt4 REQUIRED)

include(FindPkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(DBUSMENUGLIB dbusmenu-glib-0.4)
pkg_check_modules(NM REQUIRED libnm-glib libnm-util)
add_definitions(-DQT_NO_KEYWORDS)

add_subdirectory(server)
add_subdirectory(client)

find_program(DBUS_RUNNER
             dbus-test-runner)

if(NOT DBUS_RUNNER)
    message(STATUS "dbus-test-runner not found tests disabled.")
else()
    enable_testing()
    add_subdirectory(tests)
endif()

include(FindDoxygen)
if(NOT DOXYGEN_FOUND)
    message(STATUS "doxygen not fount documention disabled.")
endif()

