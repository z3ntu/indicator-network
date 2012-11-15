#SUBDIRS = data

#check_LIBRARIES = libgtest.a
#check_PROGRAMS = \
#	test-exportmenu
#nodist_libgtest_a_SOURCES = \
#	$(GTEST_SOURCE)/src/gtest-all.cc
#libgtest_a_CPPFLAGS = $(GTEST_CPPFLAGS)
#libgtest_a_CXXFLAGS = $(GTEST_CPPFLAGS)
#TESTS = $(check_PROGRAMS)
#TESTS_ENVIRONMENT = XDG_DATA_DIRS=$(builddir)/data:/usr/share
#test_exportmenu_SOURCES = \
#	test-exportmenu.cc
#test_exportmenu_CXXFLAGS = $(GLIB_CFLAGS) $(GTEST_CPPFLAGS) -I$(top_srcdir)/exportmenu
#test_exportmenu_LDFLAGS = $(GLIB_LIBS)
#test_exportmenu_LDADD = $(top_builddir)/exportmenu/libexportmenu.la libgtest.a

TESTS =

if HAVE_DBUSMOCK
TESTS += network.py
endif
