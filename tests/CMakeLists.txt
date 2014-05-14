
set(CMAKE_AUTOMOC OFF)
include(FindGMock)
set(CMAKE_AUTOMOC ON)

find_package("Valgrind" REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${GMOCK_INCLUDE_DIRS})
include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(autopilot)
add_subdirectory(integration)
add_subdirectory(data)
add_subdirectory(unit)
add_subdirectory(utils)
