bin_PROGRAMS = exportmenu


############### Export menu ###############
exportmenu_SOURCES =      \
	Parser.vala       \
	Settings.vala     \
	MenuExporter.vala \
	ExportMenu.vala 

exportmenu_LDFLAGS = \
	$(GLIB_LIBS) \
	$(DBUSMENU_LIBS)

exportmenu_CFLAGS =    \
	$(GLIB_CFLAGS) \
	$(DBUSMENU_CFLAGS)

exportmenu_VALAFLAGS = --pkg Dbusmenu-0.4 --pkg gio-2.0
