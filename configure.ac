
AC_INIT(indicator-network, 0.5, ted@canonical.com)
AC_COPYRIGHT([Copyright 2011,2012,2013 Canonical])

AC_PREREQ(2.62)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(indicator-network, 0.5.0)

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

##################################################
# Option to allow distcheck to run smoothly
##################################################
AC_ARG_WITH([localenabled],
            [AC_HELP_STRING([--with-localenabled],
                            [Enable local install instead of a system install of files.])],
            [],
            [with_localenabled=no])


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

DBUS_REQUIRED_VERSION=1.6
PKG_CHECK_MODULES(DBUS, dbus-1 >= $DBUS_REQUIRED_VERSION)

if test "x$with_localenabled" = xno;
then
  DBUS_SERVICE_DIR=`pkg-config dbus-1 --variable=session_bus_services_dir`
else
  DBUS_SERVICE_DIR=${datadir}/dbus-1/services
fi

AC_SUBST(DBUS_SERVICE_DIR)

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

###########################
# Vala Compiler
###########################

AM_PROG_VALAC(0.17.4)

############################
# GSettings schema compiler
############################

AC_PATH_PROG([GSCHEMA_COMPILER], [glib-compile-schemas])
AM_CONDITIONAL([HAVE_GSCHEMA_COMPILER], [test -n "$GSCHEMA_COMPILER"])

###########################
# Google test
###########################

AC_ARG_ENABLE([tests],
    [AS_HELP_STRING([--disable-tests], [Disable test scripts and tools (default=auto)])],
    [enable_tests=${enableval}],
    [enable_tests=auto])
if test "x$enable_tests" != "xno"; then
    m4_include([m4/gtest.m4])
    CHECK_GTEST
    if test "x$enable_tests" = "xauto"; then
        enable_tests=${have_gtest}
    elif test "x$enable_tests" = "xyes" && test "x$have_gtest" != "xyes"; then
        AC_MSG_ERROR([tests were requested but gtest is not installed.])
    fi
fi
AM_CONDITIONAL([BUILD_TESTS],[test "x$enable_tests" = "xyes"])

###########################
# dbusmock
###########################

AM_PATH_PYTHON(3.0,, [:])
AC_PYTHON_MODULE(dbusmock)
AM_CONDITIONAL([HAVE_DBUSMOCK],[test "x$HAVE_PYMOD_DBUSMOCK" = "xyes"])

###########################
# Files
###########################

AC_OUTPUT([
Makefile
data/Makefile
network/Makefile
secret-agent/Makefile
test/Makefile
])
