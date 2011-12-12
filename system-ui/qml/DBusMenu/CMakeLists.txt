project(DBusMenuQML)

set(DBUSMENUQML_SRC
    plugin.cpp
)

set(DBUSMENUQML_HEADERS
    plugin.h
)

qt4_wrap_cpp(DBUSMENUQML_MOC ${DBUSMENUQML_HEADERS})
add_library(dbusmenuqml MODULE ${DBUSMENUQML_SRC} ${DBUSMENUQML_MOC})

include_directories(
    ${dbusmodel_SOURCE_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTDECLARATIVE_INCLUDE_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${DBUSMENUGLIB_INCLUDE_DIRS}
)

target_link_libraries(dbusmenuqml
    dbusmodel
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${GLIB_LDFLAGS}
    ${DBUSMENUGLIB_LDFLAGS}
)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/qmldir"
                                                              "${CMAKE_CURRENT_BINARY_DIR}/qmldir")
