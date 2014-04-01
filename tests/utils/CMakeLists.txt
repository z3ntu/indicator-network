
include_directories("${CMAKE_SOURCE_DIR}/network")

add_library(
    test-utils
    STATIC
    main.cpp
    action-utils.cpp
)

qt5_use_modules(
    test-utils
    Core
)