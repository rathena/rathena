dnl Process this file with autoconf to produce a configure script.

AC_INIT([libconfig],[1.8.1],[hyperrealm@gmail.com],[libconfig],[https://hyperrealm.github.io/libconfig/])
AC_CONFIG_AUX_DIR([aux-build])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(ac_config.h)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_MSG_NOTICE([libconfig - made with pride in Colorado])
sleep 3

AC_DEFINE_UNQUOTED(TARGET, "${target}", [Configured target name.])

# Enable GNU extensions.
AC_USE_SYSTEM_EXTENSIONS

LT_INIT(win32-dll)

dnl Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX

AM_PROG_LEX(noyywrap)
if test Z"$LEX" != Zflex; then
cat <<EOF
*******************************************************************
You do not seem to have flex. While flex is not required to build
libconfig, you may need it to regenerate the lexer if you change
the scanner.l file.
*******************************************************************
EOF
fi

AC_PROG_YACC(noyywrap)
if test Z"$YACC" != "Zbison -y"; then
cat <<EOF
*******************************************************************
You do not seem to have bison. While bison is not required to build
libconfig, you may need it to regenerate the parser if you change
the grammar.y file.
*******************************************************************
EOF
fi

AC_MSG_CHECKING([for compiler switch to enable full C/C++ warnings]);

dnl Enable warnings, if we can determine an appropriate switch...
case "${CC}" in

gcc)
	warn_c_sw="-Wall -Wshadow -Wextra -Wdeclaration-after-statement -Wno-unused-parameter"
	warn_cxx_sw="-Wall -Wshadow -Wextra -Wno-unused-parameter";;
*)
	warn_cxx_sw=""
	warn_c_sw="";;
esac

if test -n "${warn_c_sw}";
then
	CFLAGS="${CFLAGS} ${warn_c_sw}";
	CXXFLAGS="${CXXFLAGS} ${warn_cxx_sw}";
	AC_MSG_RESULT([${warn_c_sw}, ${warn_cxx_sw}])
else
	AC_MSG_RESULT([(cannot determine)])
fi;


dnl Checks for header files.
AC_CHECK_INCLUDES_DEFAULT

AC_CHECK_HEADERS(unistd.h stdint.h xlocale.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for functions

AC_CHECK_FUNCS([newlocale uselocale freelocale])

dnl Package options

docxx=yes

AC_ARG_ENABLE(cxx,
AS_HELP_STRING([--disable-cxx], [Disable building of the C++ library]),
[if test "$enableval" = "no"; then docxx="no"; fi],
[
docxx=yes
]
)

AM_CONDITIONAL(BUILDCXX, test x$docxx = xyes)

dodoc=yes

AC_ARG_ENABLE(doc,
AS_HELP_STRING([--disable-doc], [Disable building of the documentation]),
[if test "$enableval" = "no"; then dodoc="no"; fi],
[
dodoc=yes
]
)

AM_CONDITIONAL(BUILDDOC, test x$dodoc = xyes)

doexamples=yes

AC_ARG_ENABLE(examples,
AS_HELP_STRING([--disable-examples], [Disable building of the example programs]),
[if test "$enableval" = "no"; then doexamples="no"; fi],
[
doexamples=yes
]
)

AM_CONDITIONAL(BUILDEXAMPLES, test x$doexamples = xyes)

dotests=yes

AC_ARG_ENABLE(tests,
AS_HELP_STRING([--disable-tests], [Disable building of the tests]),
[if test "$enableval" = "no"; then dotests="no"; fi],
[
dotests=yes
]
)

AM_CONDITIONAL(BUILDTESTS, test x$dotests = xyes)

dnl Check for MinGW. Workaround for libtool's DLL_EXPORT stupidity.

case "$target" in
	*-*-cygwin* | *-*-mingw*)
		gnuwin=yes;;
esac

AM_CONDITIONAL(GNU_WIN, test x$gnuwin = xyes)


AC_CONFIG_FILES([Makefile
	lib/Makefile
	lib/libconfig.pc
	lib/libconfig++.pc
	lib/libconfigConfig.cmake
	lib/libconfig++Config.cmake
	doc/Makefile
	examples/Makefile
	examples/c/Makefile
	examples/c++/Makefile
	tinytest/Makefile
	tests/Makefile
	libconfig.spec
	])
AC_OUTPUT
