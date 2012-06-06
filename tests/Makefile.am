SUBDIRS = data

check_PROGRAMS = \
	test-parser \
	test-exportmenu

TESTS = $(check_PROGRAMS)
TESTS_ENVIRONMENT = XDG_DATA_DIRS=$(builddir)/data:/usr/share
test_parser_SOURCES = \
	test-parser.c 

test_parser_CFLAGS = $(GLIB_CFLAGS) $(DBUSMENU_CFLAGS) -I$(top_srcdir)/exportmenu
test_parser_LDFLAGS = $(GLIB_LIBS) $(DBUSMENU_LIBS)
test_parser_LDADD = $(top_builddir)/exportmenu/libexportmenu.la

test_exportmenu_SOURCES = \
	test-exportmenu.c

test_exportmenu_CFLAGS = $(GLIB_CFLAGS) -I$(top_srcdir)/exportmenu
test_exportmenu_LDFLAGS = $(GLIB_LIBS)
test_exportmenu_LDADD = $(top_builddir)/exportmenu/libexportmenu.la
