
set(
	INDICATOR_SECRET_AGENT_SOURCES
	SecretAgent.cpp
	SecretAgentAdaptor.cpp
	SecretRequest.cpp
)

qt5_add_dbus_adaptor(
	INDICATOR_SECRET_AGENT_SOURCES
	"${DATA_DIR}/nm-secret-agent.xml"
	SecretAgentInclude.h
	SecretAgent
	SecretAgentAdaptor
)

qt5_add_dbus_interface(
	INDICATOR_SECRET_AGENT_SOURCES
	"${DATA_DIR}/nm-agent-manager.xml"
	AgentManagerInterface
)

add_library(
	indicator-secret-agent
	STATIC
	${INDICATOR_SECRET_AGENT_SOURCES}
)

qt5_use_modules(
	indicator-secret-agent
	Core
	DBus
)

add_executable(
	indicator-secret-agent-bin
	main.cpp
)

set_target_properties(
	indicator-secret-agent-bin
	PROPERTIES
	OUTPUT_NAME indicator-secret-agent
)

qt5_use_modules(
	indicator-secret-agent-bin
	Core
)

target_link_libraries(
	indicator-secret-agent-bin
	indicator-secret-agent
)

install(
	TARGETS indicator-secret-agent-bin
	RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}"
)
