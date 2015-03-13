
pkg_check_modules(
    PROPERTIESCPP REQUIRED
    properties-cpp
)
include_directories(${PROPERTIESCPP_INCLUDE_DIRS})

pkg_check_modules(
    DBUSCPP REQUIRED
    dbus-cpp
)
include_directories(${DBUSCPP_INCLUDE_DIRS})

pkg_check_modules(
    NETWORKMANAGER REQUIRED
    NetworkManager
)
include_directories(${NETWORKMANAGER_INCLUDE_DIRS})

include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_BINARY_DIR}/src/qdbus-stubs")
include_directories("${CMAKE_SOURCE_DIR}/src/qdbus-stubs")

# list these here to have them visible in Qt Creator
file(
        GLOB_RECURSE NETWORK_SERVICE_HEADERS
        RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)
add_custom_target(QtCreatorHeaderHack SOURCES ${NETWORK_SERVICE_HEADERS})

set(NETWORK_SERVICE_SOURCES
    ${NETWORK_SERVICE_HEADERS}

    nmofono/kill-switch-impl.cpp
    nmofono/manager.cpp
    nmofono/manager-impl.cpp
    nmofono/manager-impl.h
    nmofono/service-impl.cpp
    nmofono/service-impl.h
    nmofono/wifi/access-point.cpp
    nmofono/wifi/access-point-impl.cpp
    nmofono/wifi/grouped-access-point-impl.cpp
    nmofono/wifi/wifi-link-impl.cpp

    factory.cpp
    indicator-network-service.cpp
    modem.cpp
    modem-manager.cpp
    sim-unlock-dialog.cpp
    root-state.cpp
    sections/quick-access-section.cpp
    sections/wifi-section.cpp
    sections/wwan-section.cpp

    connectivity-service/connectivity-service.cpp

    menuitems/wifi-link-item.cpp
    menuitems/wwan-link-item.cpp
    menuitems/access-point-item.cpp
    menuitems/modem-info-item.cpp
)

add_library(
  indicator-network-service-static
  STATIC
  ${NETWORK_SERVICE_SOURCES}
)

target_link_libraries(
    indicator-network-service-static
    menumodel_cpp
    notify_cpp
    url_dispatcher_cpp
    qdbus-stubs
    ${DBUSCPP_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${NOTIFY_LIBRARIES}
    ${OFONO_LIBRARIES}
)

###########################
# Executable
###########################

add_executable(
  indicator-network-service
  main.cpp
)

target_link_libraries(
    indicator-network-service
    indicator-network-service-static
)


###########################
# Installation
###########################

install(
  TARGETS indicator-network-service
  RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/indicator-network/"
)
