set(INSTALL_DOCDIR ${CMAKE_INSTALL_DATAROOTDIR}/doc/connectivity-api)

if(build_type_lower MATCHES debug)
    option(BUILD_DOC "Build documentation by default" FALSE)
else()
    option(BUILD_DOC "Build documentation by default" TRUE)
endif()
option(PRIVATE_DOCS "Build private documentation" FALSE)

if(${BUILD_DOC})
    set(IS_ALL ALL)
endif()

add_subdirectory(qt)
set(DBUS_FILES
    dbus/connectivity-api.txt
)

add_custom_target(doc ${IS_ALL}
    SOURCES ${DBUS_FILES}
)
add_dependencies(doc cppdoc qmldoc)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/
    DESTINATION ${INSTALL_DOCDIR}/dbus/
)
