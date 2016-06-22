
include_directories("${CMAKE_SOURCE_DIR}/src")

add_library(
    test-utils
    STATIC
    main.cpp
    action-utils.cpp
)

target_link_libraries(
    test-utils
    Qt5::Core
    util
)
