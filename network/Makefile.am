pkglibexec_PROGRAMS = indicator-network-service
noinst_LTLIBRARIES = libnetwork.la
noinst_HEADERS     = libnetwork.h

libnetwork_la_SOURCES = \
	ofono.vala \
	url-dispatcher.vapi \
	network-action-manager.vala \
	network-menu.vala \
	device-base.vala \
	device-ethernet.vala \
	device-mobile.vala \
	device-wifi.vala \
	action-muxer.c \
	action-muxer.h \
	action-muxer.vapi \
	settings-airplane.vala \
	settings-base.vala \
	settings-wifi.vala

indicator_network_service_SOURCES = \
	config.vapi \
	network-menu-service.vala    \
	libnetwork.vapi \
	action-muxer.vapi

libnetwork.vapi: libnetwork.la
libnetwork.h: libnetwork.la

libnetwork_la_VALAFLAGS = --pkg libnm-glib --library=libnetwork --vapi=libnetwork.vapi --header=libnetwork.h
indicator_network_service_VALAFLAGS = --pkg libnm-glib

indicator_network_service_CFLAGS  = \
            -DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\" \
            -DLOCALE_DIR=\"$(datadir)/locale\" \
			$(GLIB_CFLAGS) \
			$(NM_CFLAGS) \
			$(COVERAGE_CFLAGS)
indicator_network_service_LDFLAGS   = \
			$(GLIB_LIBS) \
			$(NM_LIBS) \
			$(COVERAGE_LDFLAGS)

indicator_network_service_LDADD = \
			libnetwork.la

libnetwork_la_CFLAGS = \
            -DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\" \
            -DLOCALE_DIR=\"$(datadir)/locale\" \
			$(indicator_network_service_CFLAGS) \
			$(COVERAGE_CFLAGS)
libnetwork_la_LDFLAGS = \
			$(indicator_network_service_LDFLAGS) \
			$(COVERAGE_LDFLAGS)
