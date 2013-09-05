
add_definitions(
  -w
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

###########################
# libnetwork
###########################

vala_init(libnetwork
  PACKAGES
    libnm-glib
    libnotify
  CUSTOM_VAPIS
    "${CMAKE_CURRENT_SOURCE_DIR}/action-muxer.vapi"
    "${CMAKE_CURRENT_SOURCE_DIR}/url-dispatcher.vapi"
)

vala_add(libnetwork
  ofono.vala
)

vala_add(libnetwork
  network-action-manager.vala
  DEPENDS
    ofono
)

vala_add(libnetwork
  network-menu.vala
  DEPENDS
    device-base
    device-ethernet
    device-mobile
    device-wifi
    network-action-manager
)

vala_add(libnetwork
  device-base.vala
)

vala_add(libnetwork
  device-ethernet.vala
  DEPENDS
    device-base
)

vala_add(libnetwork
  device-mobile.vala
  DEPENDS
    device-base
)

vala_add(libnetwork
  device-wifi.vala
  DEPENDS
    device-base
)

vala_finish(libnetwork
  SOURCES
    LIBNETWORK_VALA_SOURCES
  OUTPUTS
    LIBNETWORK_VALA_C
  GENERATE_HEADER
    libnetwork.h
  GENERATE_VAPI
    libnetwork.vapi
  GENERATE_SYMBOLS
    libnetwork.def
)

set_source_files_properties(
  ${LIBNETWORK_VALA_SOURCES}
  PROPERTIES
  HEADER_FILE_ONLY TRUE
)

set(
  LIBNETWORK_SOURCES
  ${LIBNETWORK_VALA_SOURCES}
  ${LIBNETWORK_VALA_C}
  action-muxer.c
)

add_library(
  network
  STATIC
  ${LIBNETWORK_SOURCES}
)

target_link_libraries(
  network
  ${GLIB_LIBRARIES}
  ${NM_LIBRARIES}
  ${NOTIFY_LIBRARIES}
)

###########################
# network service
###########################

vala_init(network-service
  PACKAGES
    libnm-glib
    libnotify
    libnetwork
  DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/libnetwork.vapi"
  CUSTOM_VAPIS
    "${CMAKE_CURRENT_SOURCE_DIR}/config.vapi"
    "${CMAKE_CURRENT_SOURCE_DIR}/action-muxer.vapi"
  OPTIONS
    --vapidir=.
)

vala_add(network-service
  network-menu-service.vala
)

vala_finish(network-service
  SOURCES
    NETWORK_SERVICE_VALA_SOURCES
  OUTPUTS
    NETWORK_SERVICE_VALA_C
)

set_source_files_properties(
  ${NETWORK_SERVICE_VALA_SOURCES}
  PROPERTIES
  HEADER_FILE_ONLY TRUE
)

set(
  NETWORK_SERVICE_SOURCES
  ${NETWORK_SERVICE_VALA_SOURCES}
  ${NETWORK_SERVICE_VALA_C}
)

add_executable(
  indicator-network-service
  ${NETWORK_SERVICE_SOURCES}
)

target_link_libraries(
  indicator-network-service
  network
)

###########################
# Installation
###########################

install(
  TARGETS indicator-network-service
  RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/indicator-network/"
)

