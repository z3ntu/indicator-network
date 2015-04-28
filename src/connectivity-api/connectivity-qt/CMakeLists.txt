
include_directories("${CMAKE_SOURCE_DIR}/src/qdbus-stubs")

set(
    CONNECTIVITY_QT_SRC
    connectivityqt/internal/dbus-property-cache.cpp
    connectivityqt/connectivity.cpp
)

set_source_files_properties(
    "${DATA_DIR}/org.freedesktop.DBus.Properties.xml"
    "${DATA_DIR}/com.ubuntu.connectivity1.NetworkingStatus.xml"
    "${DATA_DIR}/com.ubuntu.connectivity1.Private.xml"
    PROPERTIES
    NO_NAMESPACE YES
)


qt5_add_dbus_interface(
    CONNECTIVITY_QT_SRC
    "${DATA_DIR}/com.ubuntu.connectivity1.NetworkingStatus.xml"
    NetworkingStatusInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_QT_SRC
    "${DATA_DIR}/com.ubuntu.connectivity1.Private.xml"
    NetworkingStatusPrivateInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_QT_SRC
    "${DATA_DIR}/org.freedesktop.DBus.Properties.xml"
    PropertiesInterface
)

add_library(
    connectivity-qt
    SHARED
    ${CONNECTIVITY_QT_SRC}
)

qt5_use_modules(
    connectivity-qt
    Core
    DBus
)

target_link_libraries(
    connectivity-qt
    qdbus-stubs
)
