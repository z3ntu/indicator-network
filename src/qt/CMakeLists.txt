set(CMAKE_AUTOMOC ON)
add_definitions(-DQT_NO_KEYWORDS)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

add_subdirectory(src)
add_subdirectory(qml)
