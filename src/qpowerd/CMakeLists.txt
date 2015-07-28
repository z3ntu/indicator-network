include_directories("${CMAKE_SOURCE_DIR}/src/")
include_directories("${CMAKE_SOURCE_DIR}/src/qdbus-stubs")
include_directories("${CMAKE_BINARY_DIR}/src/qdbus-stubs")

set(QPOWERD_SOURCES
    qpowerd.cpp
)

add_library(qpowerd STATIC ${QPOWERD_SOURCES})

target_link_libraries(
    qpowerd
    Qt5::Core
    Qt5::DBus
    qdbus-stubs
)
