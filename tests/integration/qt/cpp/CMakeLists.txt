
include_directories(${CMAKE_SOURCE_DIR}/src/qt/include)
set(TEST_SRCS
    main.cpp
    tst_networking.cpp
)

set(testCommand dbus-test-runner -t ${CMAKE_CURRENT_BINARY_DIR}/cpptest
    -p -o -p ${CMAKE_BINARY_DIR}/cpptest.xml,xunitxml
    -p -o -p -,txt)

add_executable(cpptest ${TEST_SRCS})
target_link_libraries(cpptest ${CONNECTIVITY_QT_LIB_TARGET})
qt5_use_modules(cpptest Core Test)

add_test(NAME cpp COMMAND ${testCommand})
