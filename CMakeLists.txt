project(indicator-network C CXX)
cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")


# combine these two?
set(PACKAGE ${CMAKE_PROJECT_NAME})
set(GETTEXT_PACKAGE indicator-network)
set(LOCALE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/locale")

# in config.h
#add_definitions( -DGETTEXT_PACKAGE="${GETTEXT_PACKAGE}" )

find_package(PkgConfig REQUIRED)
include(GNUInstallDirs)
include(Coverage)
include(UseGSettings)

# Workaround for libexecdir on debian
if (EXISTS "/etc/debian_version")
  set(CMAKE_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_LIBDIR})
  set(CMAKE_INSTALL_FULL_LIBEXECDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR}")
endif()

set(DATA_DIR "${CMAKE_SOURCE_DIR}/data")

set(GLIB_REQUIRED_VERSION 2.26)

pkg_check_modules(
  GLIB REQUIRED
  glib-2.0>=${GLIB_REQUIRED_VERSION}
  gio-2.0>=${GLIB_REQUIRED_VERSION}
  url-dispatcher-1
)
include_directories(${GLIB_INCLUDE_DIRS})

set(NM_REQUIRED_VERSION 0.9)
pkg_check_modules(
  NM REQUIRED
  libnm-glib>=${NM_REQUIRED_VERSION}
  libnm-util>=${NM_REQUIRED_VERSION}
)
include_directories(${NM_INCLUDE_DIRS})

set(OFONO_REQUIRED_VERSION 1.12)
pkg_check_modules(
  OFONO REQUIRED
  ofono>=${OFONO_REQUIRED_VERSION}
)
include_directories(${OFONO_INCLUDE_DIRS})


set(NOTIFY_REQUIRED_VERSION 0.7.5)
pkg_check_modules(
  NOTIFY REQUIRED
  libnotify>=${NOTIFY_REQUIRED_VERSION}
)
include_directories(${NOTIFY_INCLUDE_DIRS})

pkg_check_modules(
    CONNECTIVITY_CPP REQUIRED
    connectivity-cpp
)
include_directories(${CONNECTIVITY_CPP_INCLUDE_DIRS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

find_package(Qt5DBus COMPONENTS Qt5DBusMacros REQUIRED)
include_directories(${Qt5DBus_INCLUDE_DIRS})

find_package(Qt5Test REQUIRED)
include_directories(${Qt5Test_INCLUDE_DIRS})

pkg_check_modules(NM REQUIRED NetworkManager REQUIRED)
include_directories(${NM_INCLUDE_DIRS})

pkg_check_modules(QTDBUSTEST REQUIRED libqtdbustest-1 REQUIRED)
include_directories(${QTDBUSTEST_INCLUDE_DIRS})

pkg_check_modules(QTDBUSMOCK REQUIRED libqtdbusmock-1 REQUIRED)
include_directories(${QTDBUSMOCK_INCLUDE_DIRS})

pkg_check_modules(QMENUMODEL REQUIRED qmenumodel REQUIRED)
include_directories(${QMENUMODEL_INCLUDE_DIRS})

pkg_check_modules(GIO REQUIRED gio-2.0>=${GLIB_REQUIRED_VERSION})
include_directories(${GIO_INCLUDE_DIRS})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(GObjectIntrospection 0.9.12)

include_directories(${CMAKE_BINARY_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(
    -Wall
    -Wextra
    -Wpedantic
)

configure_file(
  "config.h.in"
  "config.h"
)

add_subdirectory(data)
add_subdirectory(network)
add_subdirectory(secret-agent)
add_subdirectory(po)

enable_testing()
add_subdirectory(tests)

ADD_CUSTOM_TARGET(
        check
        ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure
)

