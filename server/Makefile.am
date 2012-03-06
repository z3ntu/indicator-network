bin_PROGRAMS =         \
	exportmenu     \
	networkservice

noinst_HEADERS = accesspointitem.h accesspoint-info.h

############### Export menu ###############
exportmenu_SOURCES =      \
	Parser.vala       \
	Settings.vala     \
	MenuExporter.vala

exportmenu_LDFLAGS = \
	$(GLIB_LIBS) \
	$(DBUSMENU_LIBS)

exportmenu_CFLAGS =    \
	$(GLIB_CFLAGS) \
	$(DBUSMENU_CFLAGS)

exportmenu_VALAFLAGS = --pkg Dbusmenu-0.4 --pkg gio-2.0

############## Network Service ############
networkservice_SOURCES =     \
	Settings.vala        \
	MenuExporter.vala    \
	network-service.c    \
	network-utils-wifi.c \
	accesspointitem.c    \
	accesspoint-info.c

networkservice_VALAFLAGS = --pkg Dbusmenu-0.4 --pkg gio-2.0

networkservice_LDFLAGS = \
	$(GLIB_LIBS)     \
	$(DBUSMENU_LIBS) \
	$(NM_LIBS)

networkservice_CFLAGS =    \
	$(GLIB_CFLAGS)     \
	$(DBUSMENU_CFLAGS) \
	$(NM_CFLAGS)
