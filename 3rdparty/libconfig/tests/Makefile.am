
check_PROGRAMS = libconfig_tests
noinst_PROGRAMS=$(check_PROGRAMS)
TESTS = $(check_PROGRAMS)

libconfig_tests_SOURCES = tests.c

libconfig_tests_CPPFLAGS = -I$(top_srcdir)/tinytest -I$(top_srcdir)/lib

libconfig_tests_LDADD = -L$(top_builddir)/tinytest -ltinytest \
	-L$(top_builddir)/lib/.libs -lconfig


EXTRA_DIST = \
	tests.vcproj \
	tests.vcxproj \
	testdata/*.cfg \
	testdata/*.txt \
	CMakeLists.txt
