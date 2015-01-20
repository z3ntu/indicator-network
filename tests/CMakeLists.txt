
include(FindGMock)

find_package("Valgrind" REQUIRED)

set(CMAKE_AUTOMOC ON)

function(add_dbus_test NAME)
        add_test(
                ${NAME}
                qdbus-simple-test-runner "${CMAKE_CURRENT_BINARY_DIR}/${NAME}"
        )

        set_tests_properties(
                ${NAME}
                PROPERTIES ENVIRONMENT "GTEST_OUTPUT=xml:${CMAKE_CURRENT_BINARY_DIR}/"
        )
endfunction()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${GMOCK_INCLUDE_DIRS})
include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(autopilot)
add_subdirectory(integration)
add_subdirectory(data)
add_subdirectory(unit)
add_subdirectory(utils)
