upstartjobdir = $(datadir)/upstart/sessions/
upstartjob_DATA = \
	indicator-network.conf

EXTRA_DIST = \
	    indicator-network.conf.in

CLEANFILES = $(upstartjob_DATA)

indicator-network.conf: indicator-network.conf.in Makefile
	sed -e 's|@NETWORK_SERVICE[@]|$(pkglibdir)/indicator-network-menu-server|g' $< >$@

indicatordir = $(INDICATOR_DIR)
dist_indicator_DATA = network.indicator

EXTRA_DIST += $(dist_indicator_DATA)
