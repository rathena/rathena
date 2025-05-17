
check_PROGRAMS = example1 example2 example3 example4
AM_DEFAULT_SOURCE_EXT = .cpp

noinst_PROGRAMS=$(check_PROGRAMS)
TESTS = $(check_PROGRAMS)
XFAIL_TESTS = example4

AM_CPPFLAGS = -I $(top_srcdir)/lib

LDADD = $(top_builddir)/lib/libconfig++.la

EXTRA_DIST = \
	example.cfg invalid.cfg \
	*.vcproj \
	*.vcxproj \
	CMakeLists.txt
