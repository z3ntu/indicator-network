
include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_BINARY_DIR}/src/qdbus-stubs")
include_directories("${CMAKE_SOURCE_DIR}/src/qdbus-stubs")

set(AGENT_SOURCES
  CredentialStore.cpp
  KeyringCredentialStore.cpp
  SecretAgent.cpp
  SecretRequest.cpp
  PasswordMenu.cpp
)

qt5_add_dbus_adaptor(
    AGENT_SOURCES
    "${DATA_DIR}/nm-secret-agent.xml"
    "agent/SecretAgentInclude.h"
    "agent::SecretAgent"
    SecretAgentAdaptor
)

add_library(
  agent-static
  STATIC
  ${AGENT_SOURCES}
)

target_link_libraries(
    agent-static
    notify_cpp
    util
    Qt5::Core
    Qt5::DBus
    ${LIBSECRET_LDFLAGS}
)

###########################
# Executables
###########################

add_executable(
  indicator-network-secret-agent
  agent-main.cpp
)

target_link_libraries(
    indicator-network-secret-agent
    agent-static
    Qt5::Core
    Qt5::DBus
)

###########################
# Installation
###########################

install(
  TARGETS
    indicator-network-secret-agent
  RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/indicator-network/"
)
