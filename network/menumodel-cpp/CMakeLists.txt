
set(MENUMODEL_CPP_SOURCES
    gio-helpers/util.cpp
    gio-helpers/variant.h

    action.cpp
    action-group.h
    action-group-exporter.h
    action-group-merger.h
    menu.h
    menu-exporter.h
    menu-item.h
    menu-merger.h
    menu-model.h
)

add_library(menumodel_cpp STATIC ${MENUMODEL_CPP_SOURCES})
target_link_libraries(
    menumodel_cpp
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
)

###########################
# Installation
###########################

#install(
#  TARGETS indicator-network-service
#  RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/indicator-network/"
#)

