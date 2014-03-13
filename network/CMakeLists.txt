set(NETWORK_SERVICE_SOURCES
    indicator-network-service.cpp
    service.h
    indicator-menu.h
    wifi-link-item.h
    wifi-settings.h
    wired-link.h
    wwan-link-item.h

    menuitems/item.h
    menuitems/access-point-item.h
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
    menumodel-cpp/util.h
    menumodel-cpp/variant.h
)

#include_directories(include)

add_executable(
  indicator-network-service
  ${NETWORK_SERVICE_SOURCES}
)
target_link_libraries(
    indicator-network-service
    ${CONNECTIVITY_CPP_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${NOTIFY_LIBRARIES}
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

