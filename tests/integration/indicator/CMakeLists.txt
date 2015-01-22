
add_definitions(-DNETWORK_SERVICE_BIN="${CMAKE_BINARY_DIR}/src/indicator/indicator-network-service")

set(
    INTEGRATION_TESTS_SRC
    menuharness/MatchResult.cpp
    menuharness/MenuItemMatcher.cpp
    menuharness/MenuMatcher.cpp
    TestIndicatorNetworkService.cpp
)

add_executable(
    integration-tests
    ${INTEGRATION_TESTS_SRC}
)

qt5_use_modules(
    integration-tests
    Core
    DBus
    Test
)

target_link_libraries(
    integration-tests
    test-utils
    ${QTDBUSMOCK_LIBRARIES}
    ${QTDBUSTEST_LIBRARIES}
    ${QMENUMODEL_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
)

add_dbus_test(
    integration-tests
    integration-tests
)
