SUBDIRS = data

check_PROGRAMS = \
		 test-parser

noinst_HEADERS = unity-settings.h

TESTS = $(check_PROGRAMS)
TESTS_ENVIRONMENT = XDG_DATA_DIRS=$(builddir)/data:/usr/share
test_parser_SOURCES =                              \
	test-parser.c                              \
	$(top_srcdir)/exportmenu/Parser.vala       \
	$(top_srcdir)/exportmenu/MenuExporter.vala \
	$(top_srcdir)/exportmenu/Settings.vala 

test_parser_VALAFLAGS = --pkg Dbusmenu-0.4 --pkg gio-2.0 -H unity-settings.h

test_parser_CFLAGS =       \
	$(GLIB_CFLAGS)     \
	$(DBUSMENU_CFLAGS)

test_parser_LDFLAGS = \
	$(GLIB_LIBS)  \
	$(DBUSMENU_LIBS)
