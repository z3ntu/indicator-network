dbusservicedir = $(datadir)/dbus-1/services
dbusservice_DATA = \
	com.canonical.settings.network.service \
	com.canonical.settings.sound.service

EXTRA_DIST = \
	    com.canonical.settings.network.service.in \
	    com.canonical.settings.sound.service.in

CLEANFILES = $(dbusservice_DATA)

com.canonical.settings.network.service: com.canonical.settings.network.service.in Makefile
	sed -e 's|@NETWORK_SERVICE[@]|$(pkglibdir)/chewie-network-menu-server|g' $< >$@

com.canonical.settings.sound.service: com.canonical.settings.sound.service.in Makefile
	sed -e 's|@SOUND_SERVICE[@]|$(pkglibdir)/chewie-sound-menu-server|g ' $< >$@

