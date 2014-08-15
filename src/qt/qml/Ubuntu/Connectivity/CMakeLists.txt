# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Quick REQUIRED)

set(CORE_SRCS
    networking-status.cpp
    plugin.cpp
)
include_directories(${CMAKE_SOURCE_DIR}/src/qt/include)

add_library(connectivity-qml SHARED
    ${CORE_SRCS}
)
target_link_libraries(connectivity-qml ${CONNECTIVITY_QT_LIB_TARGET})

qt5_use_modules(connectivity-qml Qml)

get_target_property(QMAKE_EXECUTABLE Qt5::qmake LOCATION)
exec_program(${QMAKE_EXECUTABLE} ARGS "-query QT_INSTALL_QML" OUTPUT_VARIABLE QT_IMPORTS_DIR)
file(TO_CMAKE_PATH "${QT_IMPORTS_DIR}" QT_IMPORTS_DIR)

install(
  TARGETS connectivity-qml
  ARCHIVE DESTINATION ${QT_IMPORTS_DIR}/Ubuntu/Connectivity
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION ${QT_IMPORTS_DIR}/Ubuntu/Connectivity
)

install(
  FILES qmldir
  DESTINATION ${QT_IMPORTS_DIR}/Ubuntu/Connectivity
)

# copy qmldir file into build directory for shadow builds
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/qmldir"
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
    )
