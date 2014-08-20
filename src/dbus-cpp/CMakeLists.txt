
set(DBUS_CPP_SERVICES
    services/ofono.h
)

add_custom_target(DBusCppServicesHack SOURCES ${DBUS_CPP_SERVICES})
