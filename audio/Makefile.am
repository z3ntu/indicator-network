bin_PROGRAMS = chewie-sound-menu-server
#noinst_HEADERS

############### Sound Service ##############
bindir = ${libdir}/chewie

chewie_sound_menu_server_SOURCES =               \
	AudioMenuService.vala

#	$(top_srcdir)/exportmenu/Settings.vala     
#	$(top_srcdir)/exportmenu/MenuExporter.vala 

chewie_sound_menu_server_VALAFLAGS = --pkg libpulse --pkg libpulse-mainloop-glib

chewie_sound_menu_server_LDFLAGS = \
	$(GLIB_LIBS)                 \
	$(PULSE_LIBS)

chewie_sound_menu_server_CFLAGS =    \
	$(GLIB_CFLAGS)                 \
	$(PULSE_CFLAGS)
