
include_directories("${CMAKE_SOURCE_DIR}/src/")

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
    wwan-link-item.cpp
    wwan-section.cpp

    connectivity-service/connectivity-service.cpp

    menuitems/access-point-item.h
    menuitems/item.h
    menuitems/modem-info-item.cpp
    menuitems/section.h
    menuitems/switch-item.h
    menuitems/text-item.h
)

# get std::thread working...
# http://gcc.gnu.org/bugzilla/show_bug.cgi?id=52681
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(
  indicator-network-service
  ${NETWORK_SERVICE_SOURCES}
)
target_link_libraries(
    indicator-network-service
    menumodel_cpp
    notify_cpp
    url_dispatcher_cpp
    ${CONNECTIVITY_CPP_LDFLAGS}
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${NOTIFY_LIBRARIES}
    ${OFONO_LIBRARIES}
)


###########################
# Installation
###########################

install(
  TARGETS indicator-network-service
  RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/indicator-network/"
)

