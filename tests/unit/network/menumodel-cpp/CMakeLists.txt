
set(
	MENUMODEL_CPP_UNIT_TESTS_SRC
	test-menu-exporter.cpp
	"${CMAKE_SOURCE_DIR}/tests/main.cpp"
)

add_executable(
	menumodel-cpp-unit-tests
	${MENUMODEL_CPP_UNIT_TESTS_SRC}
)

qt5_use_modules(
	menumodel-cpp-unit-tests
	Core
	DBus
	Test
)

target_link_libraries(
	menumodel-cpp-unit-tests
	${QTDBUSMOCK_LDFLAGS}
	${QTDBUSTEST_LDFLAGS}
	${QMENUMODEL_LDFLAGS}
	${GIO_LDFLAGS}
	${GTEST_LIBRARIES}
	${GMOCK_LIBRARIES}
)

add_valgrind_test(
	menumodel-cpp-unit-tests
	menumodel-cpp-unit-tests
)
