check-am: data/glib-2.0/schemas/gschemas.compiled

check_PROGRAMS = \
		 test-parser

TESTS = $(check_PROGRAMS)
TESTS_ENVIRONMENT = XDG_DATA_DIRS=$(top_srcdir)/tests/data:/usr/share

test_parser_SOURCES = test-parser.c

test_parser_CFLAGS = \
	$(GLIB_CFLAGS)

test_parser_LDFLAGS = \
	$(GLIB_LIBS)

data/glib-2.0/schemas/gschemas.compiled: data/glib-2.0/schemas/com.ubuntu.test.chewie.gschema.xml
	$(TESTS_ENVIRONMENT) $(GSCHEMA_COMPILER) $(srcdir)/data/glib-2.0/schemas/
