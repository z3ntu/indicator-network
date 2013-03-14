bin_PROGRAMS = indicator-network-menu-server
noinst_LTLIBRARIES = libnetwork.la
noinst_HEADERS     = libnetwork.h

libnetwork_la_SOURCES = network-action-manager.vala network-menu.vala

bindir = $(libdir)/$(PACKAGE_NAME)
indicator_network_menu_server_SOURCES = \
	network-menu-service.vala    \
	libnetwork.vapi

libnetwork.vapi: libnetwork.la
libnetwork.h: libnetwork.la

libnetwork_la_VALAFLAGS = --pkg libnm-glib --library=libnetwork --vapi=libnetwork.vapi --header=libnetwork.h
indicator_network_menu_server_VALAFLAGS = --pkg libnm-glib

indicator_network_menu_server_CFLAGS  = \
			$(GLIB_CFLAGS) \
			$(NM_CFLAGS)
indicator_network_menu_server_LDFLAGS   = \
			$(GLIB_LIBS) \
			$(NM_LIBS)

indicator_network_menu_server_LDADD = \
			libnetwork.la

libnetwork_la_CFLAGS = $(indicator_network_menu_server_CFLAGS)
libnetwork_la_LDFLAGS = $(indicator_network_menu_server_LDFLAGS)
