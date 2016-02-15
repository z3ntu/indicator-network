include_directories("${CMAKE_SOURCE_DIR}/src/")
include_directories("${CMAKE_BINARY_DIR}/src/qdbus-stubs")
include_directories("${CMAKE_SOURCE_DIR}/src/qdbus-stubs")

set(NOTIFY_CPP_SOURCES
    notification.cpp
    notification-manager.cpp
    snapdecision/sim-unlock.cpp
)

add_library(notify_cpp STATIC ${NOTIFY_CPP_SOURCES})
target_link_libraries(
    notify_cpp
    menumodel_cpp
    qdbus-stubs
)
