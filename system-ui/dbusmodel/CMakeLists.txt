project(dbusmodel)

set(CLIENTMODEL_XML ${CMAKE_CURRENT_SOURCE_DIR}/com.canonical.dbusmenu.xml)
set(CLIENTMODEL_XML_OUTPUT_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/dbusmenu.h
    ${CMAKE_CURRENT_BINARY_DIR}/dbusmenu.cpp
)

set(CLIENTMODEL_SRC
    dbuscontrol.cpp
    dbusmodel.cpp
    dbusmenutypes_p.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/dbusmenu.cpp
)

set(CLIENTMODEL_HEADERS
    dbuscontrol.h
    dbusmodel.h
    ${CMAKE_CURRENT_BINARY_DIR}/dbusmenu.h
)

add_custom_command(OUTPUT ${CLIENTMODEL_XML_OUTPUT_FILES}
                   DEPENDS ${CLIENTMODEL_XML}
                   COMMAND ${QDBUSXML2CPP_BIN} -c DBusMenuInterface -i dbusmenutypes_p.h -p dbusmenu.h:dbusmenu.cpp ${CLIENTMODEL_XML}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

qt4_wrap_cpp(CLIENTMODEL_MOC ${CLIENTMODEL_HEADERS})
add_library(dbusmodel STATIC ${CLIENTMODEL_SRC} ${CLIENTMODEL_MOC})

set_target_properties(dbusmodel PROPERTIES COMPILE_FLAGS -fPIC)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTDBUS_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
)

target_link_libraries(dbusmodel
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
)
