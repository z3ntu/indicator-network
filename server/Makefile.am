bin_PROGRAMS = \
	secretdaemon   \
	networkservice


secretdaemon_SOURCES = \
	secret-agent-daemon.c \
	secret-agent.c

secretdaemon_LDFLAGS = \
	$(GTK_LIBS)    \
	$(NM_LIBS)

secretdaemon_CFLAGS = \
	$(GTK_CFLAGS) \
	$(NM_CFLAGS)

networkservice_SOURCES =     \
	Settings.c           \
	MenuExporter.c       \
	network-service.c    \
	network-utils-wifi.c \
	accesspointitem.c    \
	accesspoint-info.c

networkservice_LDFLAGS = \
	$(GLIB_LIBS)     \
	$(DBUSMENU_LIBS) \
	$(NM_LIBS)

networkservice_CFLAGS =    \
	$(GLIB_CFLAGS)     \
	$(DBUSMENU_CFLAGS) \
	$(NM_CFLAGS)

Settings.c: Settings.vala
	$(VALAC) --ccode Settings.vala

MenuExporter.c: MenuExporter.vala Settings.vala
	$(VALAC) --ccode --pkg Dbusmenu-0.4 --pkg gio-2.0 MenuExporter.vala Settings.vala
