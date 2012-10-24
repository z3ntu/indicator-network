dbusservicedir = $(datadir)/dbus-1/services
dbusservice_DATA = com.canonical.settings.network.service


substitutions = sed -e 's|@NETWORK_SERVICE[@]|$(libdir)/$(PACKAGE_NAME)/chewie-network-menu-server|g'

com.canonical.settings.network.service: com.canonical.settings.network.service.in Makefile
	$(substitutions) $< >$@
