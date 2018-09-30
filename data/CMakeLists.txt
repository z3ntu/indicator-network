
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
# indicator-network-secret-agent upstart job
###########################

set(
  AGENT_SERVICE
  "${CMAKE_INSTALL_FULL_LIBEXECDIR}/indicator-network/indicator-network-secret-agent"
)

configure_file (
  indicator-network-secret-agent.conf.in
  indicator-network-secret-agent.conf
  @ONLY
)

install (
  FILES "${CMAKE_CURRENT_BINARY_DIR}/indicator-network-secret-agent.conf"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/upstart/sessions/"
)

###########################
# Indicator-network systemd jobs
###########################

# Uncomment when we drop Vivid
# pkg_get_variable(SYSTEMD_USER_DIR systemd systemduserunitdir)
set (SYSTEMD_USER_DIR "/usr/lib/systemd/user")
message (STATUS "${SYSTEMD_USER_DIR} is the systemd user unit file install dir")

configure_file (
  indicator-network-secret-agent.service.in
  indicator-network-secret-agent.service
  @ONLY
)

configure_file (
  indicator-network.service.in
  indicator-network.service
  @ONLY
)

install (
  FILES "${CMAKE_CURRENT_BINARY_DIR}/indicator-network-secret-agent.service"
  DESTINATION ${SYSTEMD_USER_DIR}
)

install (
  FILES "${CMAKE_CURRENT_BINARY_DIR}/indicator-network.service"
  DESTINATION ${SYSTEMD_USER_DIR}
)

###########################
# Indicator-network systemd Upstart overrides
###########################

set (SYSTEMD_UPSTART_DIR "${CMAKE_INSTALL_FULL_DATADIR}/upstart/systemd-session/upstart")
install (
  FILES indicator-network.override indicator-network-secret-agent.override
  DESTINATION ${SYSTEMD_UPSTART_DIR}
)

###########################
# GSettings
###########################

add_schema ("com.canonical.indicator.network.gschema.xml")
