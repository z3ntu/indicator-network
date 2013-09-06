
add_subdirectory(icons)

###########################
# Indicator-network service
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
# Indicator-network upstart job
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

###########################
# Secret agent upstart job
###########################

configure_file (
	indicator-secret-agent.conf.in
	indicator-secret-agent.conf
	@ONLY
)

install (
	FILES ${CMAKE_CURRENT_BINARY_DIR}/indicator-secret-agent.conf
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/upstart/sessions
)
	
