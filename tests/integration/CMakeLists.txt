
add_definitions(-DNETWORK_SERVICE_BIN="${CMAKE_BINARY_DIR}/network/indicator-network-service")

set(
	INTEGRATION_TESTS_SRC
	TestIndicatorNetworkService.cpp
	"${CMAKE_SOURCE_DIR}/tests/main.cpp"
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
	${QTDBUSMOCK_LIBRARIES}
	${QTDBUSTEST_LIBRARIES}
	${QMENUMODEL_LIBRARIES}
	${GTEST_LIBRARIES}
	${GMOCK_LIBRARIES}
)

add_valgrind_test(
	integration-tests
	integration-tests
)
