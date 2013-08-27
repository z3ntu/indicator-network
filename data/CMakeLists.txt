
add_subdirectory(icons)

###########################
# Indicator service
###########################

set(
  INDICATOR_DIR
  "${CMAKE_INSTALL_DATADIR}/unity/indicators"
  CACHE FILEPATH "Indicator directory"
)

install(
  FILES "com.canonical.indicator.network"
  DESTINATION "${INDICATOR_DIR}"
)

###########################
# Upstart job
###########################

set(
  NETWORK_SERVICE
  "${CMAKE_INSTALL_FULL_LIBEXECDIR}/indicator-network/indicator-network-service"
)

configure_file (
  indicator-network.conf.in
  indicator-network.conf
  @ONLY
)

install (
  FILES "${CMAKE_CURRENT_BINARY_DIR}/indicator-network.conf"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/upstart/sessions/"
)

