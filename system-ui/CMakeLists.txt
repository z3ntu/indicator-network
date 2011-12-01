project(systeui)

cmake_minimum_required(VERSION 2.6.0)

find_package(Qt4 REQUIRED)
find_program(QDBUSXML2CPP_BIN qdbusxml2cpp)
if(NOT QDBUSXML2CPP_BIN)
    message(FATAL_ERROR "qdbusxml2cpp not found.")
endif()

enable_testing()

add_subdirectory(dbusmodel)
add_subdirectory(qml)
add_subdirectory(examples)
add_subdirectory(tests)
