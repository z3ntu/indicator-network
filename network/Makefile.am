bin_PROGRAMS = chewie-network-menu-server
noinst_HEADERS = accesspointitem.h accesspoint-info.h

############### Network Service ##############
bindir = ${libdir}/chewie

chewie_network_menu_server_SOURCES =               \
	$(top_srcdir)/exportmenu/Settings.vala     \
	$(top_srcdir)/exportmenu/MenuExporter.vala \
	network-service.c                          \
	network-utils-wifi.c                       \
	accesspointitem.c                          \
	accesspoint-info.c

chewie_network_menu_server_VALAFLAGS = --pkg Dbusmenu-0.4 --pkg gio-2.0

chewie_network_menu_server_LDFLAGS = \
	$(GLIB_LIBS)                 \
	$(DBUSMENU_LIBS)             \
	$(NM_LIBS)

chewie_network_menu_server_CFLAGS =    \
	$(GLIB_CFLAGS)                 \
	$(DBUSMENU_CFLAGS)             \
	$(NM_CFLAGS)
