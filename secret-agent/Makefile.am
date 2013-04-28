noinst_PROGRAMS = indicator-secret-agent

bindir = $(libdir)/indicator
############## Secret daemon ##############
indicator_secret_agent_SOURCES = \
	secret-agent-daemon.c \
	secret-agent.c        \
	secret-agent.h

indicator_secret_agent_LDFLAGS = \
	$(GTK_LIBS)           \
	$(NM_LIBS)            \
	$(COVERAGE_LDFLAGS)

indicator_secret_agent_CFLAGS = \
	$(GTK_CFLAGS)        \
	$(NM_CFLAGS)         \
	$(COVERAGE_CFLAGS)
