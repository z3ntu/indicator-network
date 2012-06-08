
AC_INIT(chewie, 0.2, alberto.ruiz@canonical.com)
AC_COPYRIGHT([Copyright 2011,2012 Canonical])

AC_PREREQ(2.62)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(chewie, 0.2.1)

AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AC_STDC_HEADERS
AC_PROG_LIBTOOL

AC_SUBST(VERSION)
AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])

###########################
# Dependencies - GLib
###########################

GLIB_REQUIRED_VERSION=2.26
PKG_CHECK_MODULES(GLIB, glib-2.0 >= $GLIB_REQUIRED_VERSION
                        gio-2.0 >= $GLIB_REQUIRED_VERSION)
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

###########################
# Dependencies - Dbusmenu 
###########################

DBUSMENU_REQUIRED_VERSION=0.4
PKG_CHECK_MODULES(DBUSMENU, dbusmenu-glib-0.4 >= $DBUSMENU_REQUIRED_VERSION)
AC_SUBST(DBUSMENU_LIBS)
AC_SUBST(DBUSMENU_CFLAGS)

#################################
# Dependencies - NetworkManager
#################################

NM_REQUIRED_VERSION=0.9
PKG_CHECK_MODULES(NM, libnm-glib >= $NM_REQUIRED_VERSION
                      libnm-util >= $NM_REQUIRED_VERSION)
AC_SUBST(NM_LIBS)
AC_SUBST(NM_CFLAGS)

################################
# Dependencies - Gtk+
################################

GTK_REQUIRED_VERSION=3.0
PKG_CHECK_MODULES(GTK, gtk+-3.0 >= $GTK_REQUIRED_VERSION)
AC_SUBST(GTK_LIBS)
AC_SUBST(NM_CFLAGS)

############################
# Dependencies - PulseAudio
############################

PULSE_REQUIRED_VERSION=1.1
PKG_CHECK_MODULES(PULSE, libpulse               >= $PULSE_REQUIRED_VERSION
                         libpulse-mainloop-glib >= $PULSE_REQUIRED_VERSION)

###########################
# Vala Compiler
###########################

AM_PROG_VALAC(0.16.0)

############################
# GSettings schema compiler
############################

AC_PATH_PROG([GSCHEMA_COMPILER], [glib-compile-schemas])
AM_CONDITIONAL([HAVE_GSCHEMA_COMPILER], [test -n "$GSCHEMA_COMPILER"])

###########################
# Files
###########################


AC_OUTPUT([
Makefile
network/Makefile
exportmenu/Makefile
network-agent/Makefile
sound/Makefile
test/Makefile
test/data/Makefile
test/data/glib-2.0/Makefile
test/data/glib-2.0/schemas/Makefile
])
