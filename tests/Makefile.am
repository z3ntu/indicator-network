SUBDIRS = data

check_PROGRAMS = \
		 test-parser \
		 test-exportmenu

noinst_HEADERS = unity-settings.h


VALAFLAS = --pkg Dbusmenu-0.4 --pkg gio-2.0 -H unity-settings.h
CFLAGS   = $(GLIB_CFLAGS) $(DBUSMENU_CFLAGS)
LDFLAGS  = $(GLIB_LIBS) $(DBUSMENU_LIBS)

TESTS = $(check_PROGRAMS)
TESTS_ENVIRONMENT = XDG_DATA_DIRS=$(builddir)/data:/usr/share
test_parser_SOURCES =                              \
	test-parser.c                              \
	$(top_srcdir)/exportmenu/Parser.vala       \
	$(top_srcdir)/exportmenu/MenuExporter.vala \
	$(top_srcdir)/exportmenu/Settings.vala 

test_parser_VALAFLAGS = $(VALAFLAS)
test_parser_CFLAGS = $(CFLAGS)
test_parser_LDFLAGS = $(LDFLAGS)

test_exportmenu_SOURCES = \
	test-exportmenu.c \
	$(top_srcdir)/exportmenu/Parser.vala       \
	$(top_srcdir)/exportmenu/MenuExporter.vala \
	$(top_srcdir)/exportmenu/Settings.vala 
test_exportmenu_VALAFLAGS = $(VALAFLAS)
test_exportmenu_CFLAGS = $(CFLAGS)
test_exportmenu_LDFLAGS = $(LDFLAGS)
