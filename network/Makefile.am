bin_PROGRAMS = networkservice

noinst_HEADERS = accesspointitem.h accesspoint-info.h

############## Network Service ############
networkservice_SOURCES =     \
	$(top_srcdir)/exportmenu/Settings.vala        \
	$(top_srcdir)/exportmenu/MenuExporter.vala    \
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
