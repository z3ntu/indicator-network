macro(declare_test testname)
    qt4_automoc(${testname}.cpp)

    add_executable(${testname} ${testname}.cpp ${testMoc})
    target_link_libraries(${testname}
                          dbusmodel
                          ${QT_QTTEST_LIBRARY}
                          ${QT_QTCORE_LIBRARY}
                          ${QT_QTDBUS_LIBRARY}
                          ${QT_QTGUI_LIBRARY})

    add_test(${testname} ${testname})
endmacro(declare_test testname)


include_directories(${dbusmodel_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${QT_INCLUDE_DIR}
                    ${QT_QTTEST_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR}
                    ${QT_QTDBUS_INCLUDE_DIR}
                    ${QT_QTGUI_INCLUDE_DIR})

add_definitions(-DTEST_SUITE)

declare_test(dbusmodeltest)
