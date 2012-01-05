project(dbusmodel)

set(CLIENTMODEL_SRC
    dbuscontrol.cpp
    dbusmodel.cpp
    qdbusmenuitem.cpp)

set(CLIENTMODEL_HEADERS
    dbuscontrol.h
    dbusmodel.h
    qdbusmenuitem.h)

qt4_wrap_cpp(CLIENTMODEL_MOC ${CLIENTMODEL_HEADERS})
add_library(dbusmodel STATIC ${CLIENTMODEL_SRC} ${CLIENTMODEL_MOC})

set_target_properties(dbusmodel PROPERTIES COMPILE_FLAGS -fPIC)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${DBUSMENUGLIB_INCLUDE_DIRS})

target_link_libraries(dbusmodel
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
    ${DBUSMENUGLIB_LDFLAGS})

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/qmldbus.conf.in"
               "${CMAKE_CURRENT_BINARY_DIR}/qmldbus.conf")


add_custom_target(dbusmodeldoc ALL
                  COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/qmldbus.conf
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  SOURCES ${CLIENTMODEL_HEADERS})
