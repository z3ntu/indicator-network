macro(declare_test testname scriptname)
    qt4_automoc(${testname}.cpp)

    add_executable(${testname} ${testname}.cpp dbusmenuscript.cpp ${testMoc})
    target_link_libraries(${testname}
                          dbusmodel
                          ${QT_QTTEST_LIBRARY}
                          ${QT_QTCORE_LIBRARY}
                          ${QT_QTGUI_LIBRARY}
                          ${QT_QTDBUS_LIBRARY}
                          ${GLIB_LDFLAGS}
                          ${DBUSMENUGLIB_LDFLAGS})

    add_test(${testname}
             ${DBUS_RUNNER} 
             --task ${CMAKE_CURRENT_BINARY_DIR}/${testname} --task-name Client
             --task ${CMAKE_CURRENT_SOURCE_DIR}/${scriptname}.py --task-name Server
             --ignore-return)
endmacro(declare_test testname)

include_directories(${dbusmodel_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${QT_INCLUDE_DIR}
                    ${QT_QTTEST_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR}
                    ${QT_QTGUI_INCLUDE_DIR}
                    ${QT_QTDBUS_INCLUDE_DIR}
                    ${GLIB_INCLUDE_DIRS}
                    ${DBUSMENUGLIB_INCLUDE_DIRS})

add_definitions(-DTEST_SUITE)

declare_test(dbusmodeltest script_dbusmodeltest)
declare_test(dbusmodelsorttest script_dbusmodeltest)
