noinst_PROGRAMS = chewie-export-menu
noinst_LTLIBRARIES = libexportmenu.la
noinst_HEADERS = libexportmenu.h
libexportmenu_la_SOURCES =      \
	export-menu-parser.vala       \
	export-menu-settings.vala     \
	export-menu-exporter.vala

libexportmenu_la_LDFLAGS = \
	$(GLIB_LIBS) \
	$(DBUSMENU_LIBS)

libexportmenu_la_CFLAGS =    \
	$(GLIB_CFLAGS) \
	$(DBUSMENU_CFLAGS)

chewie_export_menu_SOURCES = \
	export-menu.vala \
	libexportmenu.vapi

chewie_export_menu_LDADD = \
	libexportmenu.la

chewie_export_menu_LDFLAGS = \
	$(GLIB_LIBS) \
	$(DBUSMENU_LIBS)

chewie_export_menu_CFLAGS =    \
	$(GLIB_CFLAGS) \
	$(DBUSMENU_CFLAGS) \
	-I $(top_srcidir)/exportmenu

libexportmenu_la_VALAFLAGS = --pkg Dbusmenu-0.4 --pkg gio-2.0 --library=libexportmenu --vapi=libexportmenu.vapi --header=libexportmenu.h
chewie_export_menu_VALAFLAGS = --pkg Dbusmenu-0.4 --pkg gio-2.0
