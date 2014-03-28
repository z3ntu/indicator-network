
include_directories("${CMAKE_SOURCE_DIR}/secret-agent")
include_directories("${CMAKE_BINARY_DIR}/secret-agent")

add_definitions(-DSECRET_AGENT_BIN="${CMAKE_BINARY_DIR}/secret-agent/indicator-secret-agent")

set(
    SECRET_AGENT_UNIT_TESTS_SRC
    TestSecretAgent.cpp
    "${CMAKE_SOURCE_DIR}/tests/main.cpp"
)

set_source_files_properties(
    "${DATA_DIR}/nm-secret-agent.xml"
    PROPERTIES
    INCLUDE "DBusTypes.h"
)

qt5_add_dbus_interface(
    SECRET_AGENT_UNIT_TESTS_SRC
    "${DATA_DIR}/nm-secret-agent.xml"
    SecretAgentInterface
)

add_executable(
    secret-agent-unit-tests
    ${SECRET_AGENT_UNIT_TESTS_SRC}
)

qt5_use_modules(
    secret-agent-unit-tests
    Core
    DBus
    Test
)

target_link_libraries(
    secret-agent-unit-tests
    indicator-secret-agent
    ${QTDBUSMOCK_LIBRARIES}
    ${QTDBUSTEST_LIBRARIES}
    ${QMENUMODEL_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
)

add_valgrind_test(
    secret-agent-unit-tests
    secret-agent-unit-tests
)
