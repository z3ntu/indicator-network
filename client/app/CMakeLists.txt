project(system_settings)

set(SYSTEM_SETTINGS_SRC
    serviceloader.cpp
    main.cpp)

set(SYSTEM_SETTINGS_HEADERS
    serviceloader.h)

set(SYSTEM_SETTINGS_RESOURCES
    systemsettings.qrc)

qt4_add_resources(SYSTEM_SETTINGS_RCC ${SYSTEM_SETTINGS_RESOURCES})
qt4_wrap_cpp(SYSTEM_SETTINGS_MOC ${SYSTEM_SETTINGS_HEADERS})
add_executable(system-settings ${SYSTEM_SETTINGS_SRC}
                               ${SYSTEM_SETTINGS_MOC}
                               ${SYSTEM_SETTINGS_RCC})

target_link_libraries(system-settings
                      dbusmodel
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTDECLARATIVE_LIBRARY})

include_directories(${dbusmodel_SOURCE_DIR}
                    ${QT_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR}
                    ${QT_QTGUI_INCLUDE_DIR}
                    ${QT_QTDECLARATIVE_INCLUDE_DIR})

