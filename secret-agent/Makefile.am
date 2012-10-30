noinst_PROGRAMS = chewie-secret-agent

bindir = $(libdir)/chewie
############## Secret daemon ##############
chewie_secret_agent_SOURCES = \
	secret-agent-daemon.c \
	secret-agent.c        \
	secret-agent.h

chewie_secret_agent_LDFLAGS = \
	$(GTK_LIBS)           \
	$(NM_LIBS)

chewie_secret_agent_CFLAGS = \
	$(GTK_CFLAGS)        \
	$(NM_CFLAGS)
