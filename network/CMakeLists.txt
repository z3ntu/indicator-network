
add_subdirectory(notify-cpp)
include_directories(notify-cpp)

set(NOTIFY_CPP_SOURCES
    notify-cpp/notification.cpp
    notify-cpp/snapdecision/sim-unlock.cpp
)

set(NETWORK_SERVICE_SOURCES
    indicator-network-service.cpp

    service.h
    indicator-menu.h
    modem.cpp
    modem.h
    modem-manager.cpp
    quick-access-section.cpp
    sim-unlock-dialog.cpp
    root-state.cpp
    wifi-link-item.h
    wifi-section.cpp
    wifi-settings.h
    wired-link.h
    wwan-link-item.cpp
    wwan-section.cpp

    dbus-cpp/services/ofono.h

    gio-helpers/util.h
    gio-helpers/variant.h

    menuitems/access-point-item.h
    menuitems/item.h
    menuitems/section.h
    menuitems/switch-item.h
    menuitems/text-item.h

    menumodel-cpp/action.h
    menumodel-cpp/action-group.h
    menumodel-cpp/action-group-exporter.h
    menumodel-cpp/action-group-merger.h
    menumodel-cpp/menu.h
    menumodel-cpp/menu-exporter.h
    menumodel-cpp/menu-item.h
    menumodel-cpp/menu-merger.h
    menumodel-cpp/menu-model.h

    ${NOTIFY_CPP_SOURCES}

    url-dispatcher-cpp/url-dispatcher.cpp
)

#include_directories(include)

# get std::thread working...
# http://gcc.gnu.org/bugzilla/show_bug.cgi?id=52681
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(
  indicator-network-service
  ${NETWORK_SERVICE_SOURCES}
)
target_link_libraries(
    indicator-network-service
    ${CONNECTIVITY_CPP_LDFLAGS}
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${NOTIFY_LIBRARIES}
    ${OFONO_LIBRARIES}
)

#target_link_libraries(
#  indicator-network-service
#  network
#)

###########################
# Installation
###########################

install(
  TARGETS indicator-network-service
  RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/indicator-network/"
)

