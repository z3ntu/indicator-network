bin_PROGRAMS = chewie-network-menu-server
noinst_HEADERS = network-accesspointitem.h network-accesspoint-info.h
noinst_LTLIBRARIES = libnetwork.la

############### Network Service ##############
bindir = ${libdir}/chewie

chewie_network_menu_server_SOURCES =               \
	network-menu-server.c

libnetwork_la_SOURCES =                            \
	network-utils-wifi.c                       \
	network-accesspointitem.c                  \
	network-accesspoint-info.c

chewie_network_menu_server_LDADD = \
	libnetwork.la

libnetwork_la_LDFLAGS =              \
	$(GLIB_LIBS)                 \
	$(DBUSMENU_LIBS)             \
	$(NM_LIBS)

libnetwork_la_CFLAGS =                 \
	$(GLIB_CFLAGS)                 \
	$(DBUSMENU_CFLAGS)             \
	$(NM_CFLAGS) 

chewie_network_menu_server_LDFLAGS = $(libnetwork_la_LDFLAGS)
chewie_network_menu_server_CFLAGS  = $(libnetwork_la_CFLAGS)
