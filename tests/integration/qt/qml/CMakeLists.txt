add_custom_target(qmltests)

find_program(qmltestrunner_exe qmltestrunner)

if(NOT qmltestrunner_exe)
  msg(FATAL_ERROR "Could not locate qmltestrunner.")
endif()

set(qmltest_command
  dbus-test-runner -t env -p "QT_QPA_PLATFORM=minimal"
  -p ${qmltestrunner_exe} -p -input -p ${CMAKE_CURRENT_SOURCE_DIR}/tst_api.qml
  -p -import -p ${CMAKE_BINARY_DIR}/src/qt/qml
  -p -o -p ${CMAKE_BINARY_DIR}/testapi.xml,xunitxml
  -p -o -p -,txt
)

add_custom_target(qmltest_api ${qmltest_command})
add_dependencies(qmltests qmltest_api)
add_test(qmltests ${qmltest_command})
