# list these here to have them visible in Qt Creator
set(DBUS_INTERFACES
    services/nm.h
    services/util.h
)

add_custom_target(QtCreatorHeaderHack2 SOURCES ${DBUS_INTERFACES})
