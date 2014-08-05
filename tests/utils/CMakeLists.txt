
include_directories("${CMAKE_SOURCE_DIR}/src")

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
