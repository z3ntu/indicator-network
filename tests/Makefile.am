SUBDIRS = data

check_PROGRAMS = \
		 test-parser

TESTS = $(check_PROGRAMS)
TESTS_ENVIRONMENT = XDG_DATA_DIRS=$(builddir)/data:/usr/share
test_parser_SOURCES = test-parser.c

test_parser_CFLAGS = \
	$(GLIB_CFLAGS)

test_parser_LDFLAGS = \
	$(GLIB_LIBS)
