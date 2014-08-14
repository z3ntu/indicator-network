add_subdirectory(qt)

add_custom_target(doc ALL)
add_dependencies(doc cppdoc qmldoc)

install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/
  DESTINATION ${CMAKE_INSTALL_DOCDIR}/dbus/
)
