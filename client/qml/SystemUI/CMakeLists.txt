project(SystemUI)

set(SYSTEMUI_SRC
    gradientrectangle.cpp
    pageanimation.cpp
    pagestack.cpp
    plugin.cpp
)

set(SYSTEMUI_HEADERS
    gradientrectangle.h
    pageanimation.h
    pagestack.h
    plugin.h
)

qt4_wrap_cpp(SYSTEMUI_MOC ${SYSTEMUI_HEADERS})
add_library(systemuiqml MODULE ${SYSTEMUI_SRC} ${SYSTEMUI_MOC})

include_directories(
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTDECLARATIVE_INCLUDE_DIR}
)

target_link_libraries(systemuiqml
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/qmldir"
                                                              "${CMAKE_CURRENT_BINARY_DIR}/qmldir")

add_subdirectory(components)
