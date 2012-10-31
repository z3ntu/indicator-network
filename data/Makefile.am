dbusservicedir = $(datadir)/dbus-1/services
dbusservice_DATA = \
	com.canonical.settings.network.service \
	com.canonical.settings.sound.service


network = sed -e 's|@NETWORK_SERVICE[@]|$(libdir)/$(PACKAGE_NAME)/chewie-network-menu-server|g '
sound   = sed -e 's|@SOUND_SERVICE[@]|$(libdir)/$(PACKAGE_NAME)/chewie-sound-menu-server|g '

DISTFILES = \
	    com.canonical.settings.network.service.in \
	    com.canonical.settings.sound.service.in

com.canonical.settings.network.service: com.canonical.settings.network.service.in
	$(network) $< >$@

com.canonical.settings.sound.service: com.canonical.settings.sound.service.in
	$(sound) $< >$@
