bin_PROGRAMS =         \
	secretdaemon   \
	exportmenu     \
	networkservice

############### Export menu ###############
exportmenu_SOURCES = \
	Parser.c     \
	Settings.c   \
	MenuExporter.c

exportmenu_LDFLAGS = \
	$(GLIB_LIBS) \
	$(DBUSMENU_LIBS)

exportmenu_CFLAGS =    \
	$(GLIB_CFLAGS) \
	$(DBUSMENU_CFLAGS)

############## Secret daemon ##############
secretdaemon_SOURCES =        \
	secret-agent-daemon.c \
	secret-agent.c

secretdaemon_LDFLAGS = \
	$(GTK_LIBS)    \
	$(NM_LIBS)

secretdaemon_CFLAGS = \
	$(GTK_CFLAGS) \
	$(NM_CFLAGS)

############## Network Service ############
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

Parser.c: Parser.vala
	$(VALAC) --ccode --pkg Dbusmenu-0.4 --pkg gio-2.0 MenuExporter.vala Settings.vala Parser.vala

Settings.c: Settings.vala
	$(VALAC) --ccode Settings.vala

MenuExporter.c: MenuExporter.vala Settings.vala
	$(VALAC) --ccode --pkg Dbusmenu-0.4 --pkg gio-2.0 MenuExporter.vala Settings.vala
