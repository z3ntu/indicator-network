
set(CONNECTIVITY_QT_INCLUDE_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}/connectivity-api/qt${CONNECTIVITY_QT_VERSION_MAJOR}")

set(PUBLIC_HEADER_DIR "${CMAKE_SOURCE_DIR}/src/qt/include/ubuntu/connectivity")
set(PUBLIC_HEADERS
    ${PUBLIC_HEADER_DIR}/NetworkingStatus
    ${PUBLIC_HEADER_DIR}/networking-status.h
)

#install the headers
install(FILES ${PUBLIC_HEADERS}
        DESTINATION "${CONNECTIVITY_QT_INCLUDE_DIR}/ubuntu/connectivity")

include_directories(../include)
set(CORE_SRCS
    networking-status.cpp
    dbus-properties-interface.cpp
)

add_library(${CONNECTIVITY_QT_LIB_TARGET} SHARED
    ${CORE_SRCS}
    ${PUBLIC_HEADERS} # public headers have to be included here for Q_DECL_EXPORT to work properly
)

qt5_use_modules(${CONNECTIVITY_QT_LIB_TARGET} Core DBus)

install(
  TARGETS ${CONNECTIVITY_QT_LIB_TARGET}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(SO_VERSION ${CONNECTIVITY_QT_VERSION_MAJOR})

set_target_properties(${CONNECTIVITY_QT_LIB_TARGET} PROPERTIES
                      SOVERSION ${SO_VERSION}
)

set(PC_FILE_TARGET "${CMAKE_CURRENT_BINARY_DIR}/${CONNECTIVITY_QT_LIB_TARGET}.pc")
set(libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
set(includedir "${CCONNECTIVITY_QT_INCLUDE_DIR}")
set(ABSOLUTE_SO_FILE "${CMAKE_INSTALL_FULL_LIBDIR}/${CONNECTIVITY_QT_LIB_TARGET}.so.${SO_VERSION}")
configure_file("connectivity-qt.pc.in" ${PC_FILE_TARGET} @ONLY)
install(
  FILES ${PC_FILE_TARGET}
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
