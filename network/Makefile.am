bin_PROGRAMS = chewie-network-menu-server
noinst_LTLIBRARIES = libnetwork.la
noinst_HEADERS     = libnetwork.h

libnetwork_la_SOURCES = network-action-manager.vala network-menu.vala

bindir = $(libdir)/$(PACKAGE_NAME)
chewie_network_menu_server_SOURCES = \
	network-menu-service.vala    \
	libnetwork.vapi

libnetwork.vapi: libnetwork.la
libnetwork.h: libnetwork.la

libnetwork_la_VALAFLAGS = --pkg libnm-glib --library=libnetwork --vapi=libnetwork.vapi --header=libnetwork.h
chewie_network_menu_server_VALAFLAGS = --pkg libnm-glib

chewie_network_menu_server_CFLAGS  = \
			$(GLIB_CFLAGS) \
			$(NM_CFLAGS)
chewie_network_menu_server_LDFLAGS   = \
			$(GLIB_LIBS) \
			$(NM_LIBS)

chewie_network_menu_server_LDADD = \
			libnetwork.la

libnetwork_la_CFLAGS = $(chewie_network_menu_server_CFLAGS)
libnetwork_la_LDFLAGS = $(chewie_network_menu_server_LDFLAGS)
