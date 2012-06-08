EXTRA_DIST = com.ubuntu.test.chewie.gschema.xml
BUILT_SOURCES = gschemas.compiled

DISTCLEANFILES = $(BUILT_SOURCES)
CLEANFILES = $(BUILT_SOURCES)

gschemas.compiled: com.ubuntu.test.chewie.gschema.xml
	@if test ! -f $(builddir)/com.ubuntu.test.chewie.gschema.xml ; \
	then \
		cp $(srcdir)/com.ubuntu.test.chewie.gschema.xml $(builddir) ; \
	fi
	
	XDG_DATA_DIRS=.:/usr/share $(GSCHEMA_COMPILER) $(builddir)
	
	@if test $(builddir) != $(srcdir); \
	then \
		rm -f $(builddir)/com.ubuntu.test.chewie.gschema.xml ; \
	fi
