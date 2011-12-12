project(server)

cmake_minimum_required(VERSION 2.6.0)

find_program(VALA_BIN valac)
if (NOT VALA_BIN)
    message(FATAL_ERROR "Vala compiler not found.")
endif()

include(FindPkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(DBUSMENUGLIB dbusmenu-glib-0.4)

add_subdirectory(src)
