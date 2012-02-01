project(server)

find_program(GLIB_GENMARSHAL glib-genmarshal)
if (NOT GLIB_GENMARSHAL)
  message(FATAL_ERROR "GLib genmarshal not found.")
endif()

find_program(VALA_BIN valac)
if (NOT VALA_BIN)
    message(FATAL_ERROR "Vala compiler not found.")
endif()

set(exportmenu_VALA
    ${CMAKE_CURRENT_SOURCE_DIR}/Parser.vala
    ${CMAKE_CURRENT_SOURCE_DIR}/Settings.vala
    ${CMAKE_CURRENT_SOURCE_DIR}/MenuExporter.vala)

set(exportmenu_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/Parser.c
    ${CMAKE_CURRENT_BINARY_DIR}/Settings.c
    ${CMAKE_CURRENT_BINARY_DIR}/MenuExporter.c)

set(networkservice_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/Settings.c
    ${CMAKE_CURRENT_BINARY_DIR}/MenuExporter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/network-service.c
    ${CMAKE_CURRENT_SOURCE_DIR}/network-utils-wifi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/accesspointitem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/accesspointitem.h)

set(secretmarshal_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/secret-marshal.list)

set(secretmarshal_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/secret-marshal.c)

set(secretmarshal_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/secret-marshal.h)

set(secretdaemon_SRC
    ${secretmarshal_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/secret-agent-daemon.c
    ${CMAKE_CURRENT_SOURCE_DIR}/secret-agent.c)


set(CMAKE_C_FLAGS "-Wall -g")

add_custom_command(OUTPUT ${exportmenu_SRC}
                   COMMAND ${VALA_BIN} --ccode --pkg Dbusmenu-0.4 --pkg gio-2.0 ${exportmenu_VALA}
                   DEPENDS ${exportmenu_VALA}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BIN_DIR})

add_custom_command(OUTPUT  ${secretmarshal_SRC}
                   COMMAND ${GLIB_GENMARSHAL} ${secretmarshal_LIST} --prefix=_secret_agent_marshal --body > ${secretdaemon_SRC}
                   DEPENDS ${secretmarshal_LIST} ${secretmarshal_HEADER}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BIN_DIR})

add_custom_command(OUTPUT  ${secretmarshal_HEADER}
                   COMMAND ${GLIB_GENMARSHAL} ${secretmarshal_LIST} --prefix=_secret_agent_marshal --header > ${secretmarshal_HEADER}
                   DEPENDS ${secretmarshal_LIST}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BIN_DIR})

add_executable(secret-daemon
               ${secretdaemon_SRC})

add_executable(export-settings-menu
               ${exportmenu_SRC})

add_executable(network-settings-menu
               ${networkservice_SRC})


include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${NM_INCLUDE_DIRS}
    ${DBUSMENUGLIB_INCLUDE_DIRS})

target_link_libraries(secret-daemon
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
    ${NM_LDFLAGS})

target_link_libraries(network-settings-menu
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
    ${NM_LDFLAGS}
    ${DBUSMENUGLIB_LDFLAGS})

target_link_libraries(export-settings-menu
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
    ${DBUSMENUGLIB_LDFLAGS})
