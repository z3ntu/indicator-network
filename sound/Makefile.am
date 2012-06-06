bin_PROGRAMS = chewie-sound-menu-server
noinst_LTLIBRARIES = libsound.la
noinst_HEADERS = libsound.h

############### Sound Service ##############
bindir = ${libdir}/chewie

libsound_la_SOURCES = \
	sound-menu-application.vala

chewie_sound_menu_server_SOURCES = \
	sound-menu-server.vala     \
	libsound.vapi

libsound_la_VALAFLAGS = --pkg libpulse --pkg libpulse-mainloop-glib --pkg gio-2.0 --library=libsound --vapi=libsound.vapi --header=libsound.h
chewie_sound_menu_server_VALAFLAGS = --pkg libpulse --pkg libpulse-mainloop-glib --pkg gio-2.0

chewie_sound_menu_server_LDADD = \
	libsound.la

chewie_sound_menu_server_LDFLAGS = \
	$(GLIB_LIBS)               \
	$(GIO_LIBS)                \
	$(PULSE_LIBS)

chewie_sound_menu_server_CFLAGS =    \
	$(GLIB_CFLAGS)               \
	$(GIO_CFLAGS)                \
	$(PULSE_CFLAGS)

libsound_la_LDFLAGS = $(chewie_sound_menu_server_LDFLAGS)
libsound_la_CFLAGS = $(chewie_sound_menu_server_CFLAGS)
