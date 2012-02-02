project(system-settings)

cmake_minimum_required(VERSION 2.6.0)

include(FindPkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(DBUSMENUGLIB dbusmenu-glib-0.4)
pkg_check_modules(NM REQUIRED libnm-glib libnm-util)
pkg_check_modules(GTK REQUIRED gtk+-3.0)
add_subdirectory(server)
