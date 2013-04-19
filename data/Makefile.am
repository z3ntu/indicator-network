dbusservicedir = $(DBUS_SERVICE_DIR)
dbusservice_DATA = \
	com.canonical.indicator.network.service

EXTRA_DIST = \
	    com.canonical.indicator.network.service.in

CLEANFILES = $(dbusservice_DATA)

com.canonical.indicator.network.service: com.canonical.indicator.network.service.in Makefile
	sed -e 's|@NETWORK_SERVICE[@]|$(pkglibdir)/indicator-network-menu-server|g' $< >$@

indicatordir = $(INDICATOR_DIR)
dist_indicator_DATA = network.indicator

EXTRA_DIST += $(dist_indicator_DATA)
