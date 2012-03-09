EXTRA_DIST = com.ubuntu.test.chewie.gschema.xml
BUILT_SOURCES = gschemas.compiled

DISTCLEANFILES = $(BUILT_SOURCES)
CLEANFILES = $(BUILT_SOURCES)

gschemas.compiled: com.ubuntu.test.chewie.gschema.xml
	@cp $(srcdir)/com.ubuntu.test.chewie.gschema.xml $(builddir)
	XDG_DATA_DIRS=.:/usr/share $(GSCHEMA_COMPILER) $(builddir)
	@rm -f $(builddir)/com.ubuntu.test.chewie.gschema.xml
