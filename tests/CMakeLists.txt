
set(CMAKE_AUTOMOC OFF)
include(FindGMock)
set(CMAKE_AUTOMOC ON)

find_package("Valgrind" REQUIRED)

include_directories("${CMAKE_SOURCE_DIR}/secret-agent")
include_directories("${CMAKE_BINARY_DIR}/secret-agent")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${GMOCK_INCLUDE_DIRS})
include_directories(${GTEST_INCLUDE_DIRS})

add_definitions(-DSECRET_AGENT_BIN="${CMAKE_BINARY_DIR}/secret-agent/indicator-secret-agent")
add_definitions(-DNETWORK_SERVICE_BIN="${CMAKE_BINARY_DIR}/network/indicator-network-service")

set(
	UNIT_TESTS_SRC
	TestSecretAgent.cpp
	main.cpp
)

set_source_files_properties(
	"${DATA_DIR}/nm-secret-agent.xml"
	PROPERTIES
	INCLUDE "DBusTypes.h"
)

qt5_add_dbus_interface(
	UNIT_TESTS_SRC
	"${DATA_DIR}/nm-secret-agent.xml"
	SecretAgentInterface
)

add_executable(
	unit-tests
	${UNIT_TESTS_SRC}
)

qt5_use_modules(
	unit-tests
	Core
	DBus
	Test
)

target_link_libraries(
	unit-tests
	indicator-secret-agent
	${QTDBUSMOCK_LIBRARIES}
	${QTDBUSTEST_LIBRARIES}
	${GTEST_LIBRARIES}
	${GMOCK_LIBRARIES}
)

add_valgrind_test(
	unit-tests
	unit-tests
)
