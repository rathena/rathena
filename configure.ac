#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(rAthena)
AC_REVISION($Revision$)
AC_PREREQ([2.59])
AC_LANG([C++])
AC_LANG_COMPILER_REQUIRE
AC_CONFIG_SRCDIR([src/common/cbasetypes.hpp])
AC_CONFIG_FILES([Makefile src/common/Makefile])
AC_CONFIG_FILES([3rdparty/libconfig/Makefile 3rdparty/yaml-cpp/Makefile 3rdparty/rapidyaml/Makefile 3rdparty/httplib/Makefile])
AC_CONFIG_FILES([src/char/Makefile src/login/Makefile])
AC_CONFIG_FILES([src/map/Makefile src/tool/Makefile])
AC_CONFIG_FILES([src/web/Makefile])

AC_GNU_SOURCE

#
# Memory managers
#
AC_ARG_ENABLE(
	[manager],
	AC_HELP_STRING(
		[--enable-manager=ARG],
		[memory managers: no, builtin, memwatch, dmalloc, gcollect, bcheck (defaults to builtin)]
	),
	[
		enable_manager="$enableval"
		case $enableval in
			"no");;
			"builtin");;
			"memwatch");;
			"dmalloc");;
			"gcollect");;
			"bcheck");;
			*) AC_MSG_ERROR([[unknown memory manager '$enableval'... stopping]]);;
		esac
	],
	[enable_manager="builtin"]
)


#
# packetver
#
AC_ARG_ENABLE(
	[packetver],
	AC_HELP_STRING(
		[--enable-packetver=ARG],
		[Sets the PACKETVER define. (see src/common/mmo.hpp)]
	),
	[enable_packetver="$enableval"],
	[enable_packetver=""]
)


#
# Epoll
#
AC_ARG_ENABLE(
	[epoll],
	AC_HELP_STRING(
		[--enable-epoll],
		[use epoll(4) on Linux]
	),
	[enable_epoll=$enableval],
	[enable_epoll=no]
)
if test x$enable_epoll = xno; then
	have_linux_epoll=no
else
	AC_MSG_CHECKING([for Linux epoll(4)])
	AC_LINK_IFELSE([AC_LANG_PROGRAM(
		[
		#ifndef __linux__
		#error This is not Linux
		#endif
		#include <sys/epoll.h>
		],
		[epoll_create1 (EPOLL_CLOEXEC);])],
		[have_linux_epoll=yes],
		[have_linux_epoll=no]
	)
	AC_MSG_RESULT([$have_linux_epoll])
fi
if test x$enable_epoll,$have_linux_epoll = xyes,no; then
	AC_MSG_ERROR([epoll support explicitly enabled but not available])
fi


#
# debug
#
AC_ARG_ENABLE(
	[debug],
	AC_HELP_STRING(
		[--enable-debug@<:@=ARG@:>@],
		[
			Compiles extra debug code. (disabled by default)
			(available options: yes, no, gdb)
		]
	),
	[
		enable_debug="$enableval"
		case $enableval in
			"no");;
			"yes");;
			"gdb");;
			*) AC_MSG_ERROR([[invalid argument --enable-debug=$enableval... stopping]]);;
		esac
	],
	[enable_debug="no"]
)


#
# renewal
#
AC_ARG_ENABLE(
	[prere],
	AC_HELP_STRING(
		[--enable-prere@<:@=ARG@:>@],
		[
			Compiles serv in prere mode. (disabled by default)
			(available options: yes, no)
		]
	),
	[
		enable_prere="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-prere=$enableval... stopping]]);;
		esac
	],
	[enable_prere="no"]
)


#
# VIP
#
AC_ARG_ENABLE(
	[vip],
	AC_HELP_STRING(
		[--enable-vip@<:@=ARG@:>@],
		[
			Compiles serv with vip features. (disabled by default)
			(available options: yes, no)
		]
	),
	[
		enable_vip="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-vip=$enableval... stopping]]);;
		esac
	],
	[enable_vip="no"]
)


#
# warn
#
AC_ARG_ENABLE(
	[warn],
	AC_HELP_STRING(
		[--enable-warn@<:@=ARG@:>@],
		[
			Compiles with warnings. (disabled by default)
			(available options: yes, no, extra)
		]
	),
	[
		enable_warn="$enableval"
		case $enableval in
			"no");;
			"yes");;
			"extra");;
			*) AC_MSG_ERROR([[invalid argument --enable-warn=$enableval... stopping]]);;
		esac
	],
	[enable_warn="no"]
)

#
# Buildbot
#
AC_ARG_ENABLE(
	[buildbot],
	AC_HELP_STRING(
		[--enable-buildbot@<:@=ARG@:>@],
		[(available options: yes, no)]
	),
	[
		enable_buildbot="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-buildbot=$enableval... stopping]]);;
		esac
	],
	[enable_buildbot="no"]
)

#
# RDTSC as Tick Source
#
AC_ARG_ENABLE(
	[rdtsc],
	AC_HELP_STRING(
		[--enable-rdtsc],
		[
			Uses rdtsc as timing source (disabled by default)
			Enable it when you've timing issues.
			(For example:  in conjunction with XEN or Other Virtualization mechanisms)
			Note:
				Please ensure that you've disabled dynamic CPU-Frequencys, such as power saving options.
				(On the most modern Dedicated Servers cpufreq is preconfigured, see your distribution's manual
				 how to disable it)
		]
	),
	[
		enable_rdtsc=1
	],
	[enable_rdtsc=0]
)

#
# Profiler
#
AC_ARG_ENABLE(
	[profiler],
	AC_HELP_STRING(
		[--enable-profiler=ARG],
		[Profilers: no, gprof (disabled by default)]
	),
	[
		enable_profiler="$enableval"
		case $enableval in
			"no");;
			"gprof");;
			*) AC_MSG_ERROR([[invalid argument --enable-profiler=$enableval... stopping]]);;
		esac
	],
	[enable_profiler="no"]
)


#
# 64bit
#
AC_ARG_ENABLE(
	[64bit],
	AC_HELP_STRING(
		[--disable-64bit],
		[
			Enforce 32bit output on x86_64 systems.
		]
	),
	[
		enable_64bit="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-64bit=$enableval... stopping]]);;
		esac
	],
	[enable_64bit="yes"]
)


#
# LTO
#
AC_ARG_ENABLE(
    [lto],
    AC_HELP_STRING(
        [--enable-lto],
        [
			Enables or Disables Linktime Code Optimization (LTO is enabled by default)
        ]
    ),
    [
        enable_lto="$enableval"
        case $enableval in
            "no");;
            "yes");;
            *) AC_MSG_ERROR([[invalid argument --enable-lto=$disableval... stopping]]);;
        esac
    ],
    [enable_lto="no"]
)


#
# Optionally set the maximum number of network connections
# the core will be able to handle
#
AC_ARG_WITH(
	[maxconn],
	AC_HELP_STRING(
		[--with-maxconn@<:@=ARG@:>@],
		[optionally set the maximum connections the core can handle. By default the system header value will be used.
		 This will only be the compile time limit, make sure you set the correct limit with ulimit on your OS.]
	),
	[
		if ! test "$withval" -ge 0 -o "$withval" -lt 0 2>&- ; then
			AC_MSG_ERROR([Invalid argument --with-maxconn=$withval ... stopping])
		else
			CPPFLAGS="$CPPFLAGS -DMAXCONN=$withval"
		fi
	],
	[
		CPPFLAGS="$CPPFLAGS"
	]
)


#
# output bin name
#
AC_ARG_WITH(
	[outputlogin],
	AC_HELP_STRING(
		[--with-outputlogin@<:@=ARG@:>@],
		[Specify the login-serv output name (defaults to login-server)]
	),
	[output_login="$withval"],
	[output_login="login-server"]
)
AC_SUBST([OLOG],$output_login)

AC_ARG_WITH(
	[outputchar],
	AC_HELP_STRING(
		[--with-outputchar@<:@=ARG@:>@],
		[Specify the char-serv output name (defaults to char-server)]
	),
	[output_char="$withval"],
	[output_char="char-server"]
)
AC_SUBST([OCHR],$output_char)

AC_ARG_WITH(
	[outputmap],
	AC_HELP_STRING(
		[--with-outputmap@<:@=ARG@:>@],
		[Specify the map-serv output name (defaults to map-server)]
	),
	[output_map="$withval"],
	[output_map="map-server"]
)
AC_SUBST([OMAP],$output_map)

AC_ARG_WITH(
	[outputweb],
	AC_HELP_STRING(
		[--with-outputweb@<:@=ARG@:>@],
		[Specify the web-serv output name (defaults to web-server)]
	),
	[output_web="$withval"],
	[output_web="web-server"]
)
AC_SUBST([OWEB],$output_web)


#
# Optionally specify the path to mysql_config
#
AC_ARG_WITH(
	[mysql],
	AC_HELP_STRING(
		[--with-mysql@<:@=ARG@:>@],
		[optionally specify the path to the mysql_config executable]
	),
	[
		if test "$withval" != "no" ; then
			if test ! -x "$withval" ; then
				AC_MSG_ERROR([$withval is not an executable file])
			fi
			MYSQL_CONFIG_HOME="$withval"
		fi
	]
)


#
# Manual MYSQL_CFLAGS (optional)
#
AC_ARG_WITH(
	[MYSQL_CFLAGS],
	AC_HELP_STRING(
		[--with-MYSQL_CFLAGS=ARG],
		[specify MYSQL_CFLAGS manually (instead of using "mysql_config --include")]
	),
	[
		manual_MYSQL_CFLAGS="yes"
		MYSQL_CFLAGS="$withval"
	],
	[manual_MYSQL_CFLAGS="no"]
)


#
# Manual MYSQL_LIBS (optional)
#
AC_ARG_WITH(
	[MYSQL_LIBS],
	AC_HELP_STRING(
		[--with-MYSQL_LIBS=ARG],
		[specify MYSQL_LIBS manually (instead of using "mysql_config --libs")]
	),
	[
		manual_MYSQL_LIBS="yes"
		MYSQL_LIBS="$withval"
	],
	[manual_MYSQL_LIBS="no"]
)


#
# Enable/disable PCRE and optionally specify the path (optional library)
#
AC_ARG_WITH(
	[pcre],
	AC_HELP_STRING(
		[--with-pcre@<:@=ARG@:>@],
		[use PCRE library, optionally specify the full path of pcre installation directory (by default pcre is used if found)]
	),
	[
		if test "$withval" = "no" ; then
			want_pcre="no"
		else
			want_pcre="yes"
			require_pcre="yes"
			if test "$withval" != "yes" ; then
				if test ! -d "$withval" ; then
					AC_MSG_ERROR([$withval is not a directoy])
				fi
				PCRE_HOME="$withval"
			fi
		fi
	],
	[want_pcre="yes" require_pcre="no"]
)


#
# Specify the path of the zlib library (required library)
#
AC_ARG_WITH(
	[zlib],
	AC_HELP_STRING(
		[--with-zlib=DIR],
		[root directory path of zlib installation (defaults to /usr/local or /usr if not found in /usr/local).
		 Assumes that the header files are in DIR/include and the library files are in DIR/lib]
	),
	[
		test -n "$withval" && ZLIB_HOME="$withval"
	],
	[
		ZLIB_HOME=/usr/local
		test ! -f "${ZLIB_HOME}/include/zlib.h" && ZLIB_HOME=/usr
	]
)



###############################################################################
# Check for programs and types.
#



AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CXX
AC_PATH_PROG(AR, ar)
AC_LANG([C++])

CFLAGS="$CFLAGS -pipe -ffast-math -Wall"
CPPFLAGS="$CPPFLAGS -I../common"
CXXFLAGS="$CXXFLAGS -std=c++17"
#CXXFLAGS="$CXXFLAGS -std=gnu++17"
CXXFLAG_CLEARS="-std=c++17"

AC_C_BIGENDIAN(
	[AC_MSG_ERROR([[bigendian is not supported... stopping]])],
	,
	[AC_MSG_WARN([[unable to determine endianness, only little endian is supported]])]
)


if test "$enable_64bit" = "no" ; then
	AC_MSG_CHECKING([whether pointers can be stored in ints (old code)])
	pointers_fit_in_ints="no"
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM([[static int test_array[((long int)sizeof(int)) == ((long int)sizeof(void*)) ? 1 : -1];]])],
		[pointers_fit_in_ints="yes"],
		[]
	)
	if test "$pointers_fit_in_ints" = "no" ; then
		CFLAGS="$CFLAGS -m32"
		LDFLAGS="$LDFLAGS -m32"
		AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM([[static int test_array[((long int)sizeof(int)) == ((long int)sizeof(void *)) ? 1 : -1];]])],
			[pointers_fit_in_ints="yes (with -m32)"],
			[]
		)
	fi
	AC_MSG_RESULT($pointers_fit_in_ints)
	if test "$pointers_fit_in_ints" = "no" ; then
		AC_MSG_ERROR([pointers cannot be stored in ints, required for old code... stopping])
	fi
fi


#
# check if we're producing 32bit code - so well produce binarys for at least i686 (speedup: cmovs, and cmpchg8 support)
#
AC_MSG_CHECKING([whether $CC produces 32bit code])
AC_RUN_IFELSE(
	[AC_LANG_SOURCE([
		int main(int argc, char **argv){
			if(sizeof(void*) == 4) return 0;
			else return 1;
		}
	])],
	[
		AC_MSG_RESULT([yes])
		AC_MSG_CHECKING([whether $CC supports -march=i686 (and we can run it)])
		OLD_CFLAGS="$CFLAGS"
		OLD_LDFLAGS="$LDFLAGS"
		CFLAGS="$CFLAGS -march=i686"
		LDFLAGS="$LDFLAGS -march=i686"
		AC_RUN_IFELSE(
			[AC_LANG_SOURCE([
				int main(int argc, char **argv) { return 0; }
			])],
			[
				AC_MSG_RESULT([yes])
			],
			[
				AC_MSG_RESULT([no])
				CFLAGS="$OLD_CFLAGS"
				LDFLAGS="$OLD_LDFLAGS"
			],
			[
				AC_MSG_RESULT([guessing no])
				CFLAGS="$OLD_CFLAGS"
				LDFLAGS="$OLD_LDFLAGS"
			]
		)
	],
	[
		AC_MSG_RESULT([no])
	],
	[
		AC_MSG_RESULT([guessing no])
	]
)


#
# Check if CC supports __thread attribute (Thread Local Storage)
# (Usually our OSX friends 're lacking support of it in older llvm versions ..)
#
AC_MSG_CHECKING([whether $CC supports __thread specifier (TLS)])
AC_RUN_IFELSE(
	[AC_LANG_SOURCE([
		__thread int g_Test = -1;

		int main(int argc, char **argv){
			g_Test = 0;
			return g_Test;
		}
	])],
	[
		AC_MSG_RESULT([yes])
		CPPFLAGS="$CPPFLAGS -DHAS_TLS"
	],
	[
		AC_MSG_RESULT([no])
	]
)

#
# LTO Support test
#
if test "$enable_lto" != "no" ; then
	OPT_LTO_AR=""
	OPT_LTO=""
	
	OLD_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -flto -fuse-linker-plugin -ffat-lto-objects"

	OLD_LDFLAGS="$LDFLAGS"
	LDFLAGS="$LDFLAGS -flto -fuse-linker-plugin -ffat-lto-objects"

	AC_MSG_CHECKING([whether $CC supports -ffat-lto-objects])
	AC_RUN_IFELSE(
		[AC_LANG_SOURCE([
			int main(int argc, char **argv){
				return 0;
			}
		])],
		[
			AC_MSG_RESULT([yes])
			OPT_LTO_AR="-ffat-lto-objects"
		],
		[
			AC_MSG_RESULT([no])
		],
		[
			AC_MSG_RESULT([guessing no])
		]
	)
	CFLAGS="$OLD_CFLAGS"
	LDFLAGS="$OLD_LDFLAGS"

	CFLAGS="$CFLAGS -flto -fuse-linker-plugin"
	LDFLAGS="$LDFLAGS -flto -fuse-linker-plugin"
	
	AC_MSG_CHECKING([whether $CC supports -flto -fuse-linker-plugin])
	AC_RUN_IFELSE(
		[AC_LANG_SOURCE([
			int main(int argc, char **argv){
				return 0;
			}
		])],
		[
			AC_MSG_RESULT([yes])
			OPT_LTO_AR="-flto -fuse-linker-plugin $OPT_LTO_AR"
			OPT_LTO="-flto -fuse-linker-plugin"
		],
		[
			AC_MSG_RESULT([no])
		],
		[
			AC_MSG_RESULT([guessing no])
		]
	)
	CFLAGS="$OLD_CFLAGS"
	LDFLAGS="$OLD_LDFLAGS"
fi


AC_DEFUN([AC_CHECK_COMPILER_WFLAG],
	[
		AC_MSG_CHECKING([whether $CC supports -W$1])
		OLD_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS -Werror -W$1"
		AC_COMPILE_IFELSE(
			[AC_LANG_SOURCE([int foo;])],
			[
				AC_MSG_RESULT([yes])
				CFLAGS="$OLD_CFLAGS -W$1"
				# Optionally, run a test
				if test "x$2" != "x"; then
					AC_MSG_CHECKING([whether $CC can actually use -W$1])
					CFLAGS="$OLD_CFLAGS -Werror -W$1"
					AC_COMPILE_IFELSE(
						[AC_LANG_SOURCE([$2])],
						[
							AC_MSG_RESULT([no])
							CFLAGS="$OLD_CFLAGS"
						]
						[
							AC_MSG_RESULT([yes])
							CFLAGS="$OLD_CFLAGS -W$1"
						],
					)
				fi
			],
			[
				AC_MSG_RESULT([no])
				CFLAGS="$OLD_CFLAGS"
			]
		)
	]
)

AC_DEFUN([AC_CHECK_COMPILER_WNOFLAG],
	[
		AC_MSG_CHECKING([whether $CC supports -Wno-$1])
		OLD_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS -Werror -Wno-$1"
		AC_COMPILE_IFELSE(
			[AC_LANG_SOURCE([int foo;])],
			[
				# Recent versions of gcc don't fail if -Wno-foo is not recognized
				# (unless there are also other warnings), so we also check for -Wfoo
				# which always fails if not supported
				CFLAGS="$OLD_CFLAGS -Werror -W$1"
				AC_COMPILE_IFELSE(
					[AC_LANG_SOURCE([int foo;])],
					[
						AC_MSG_RESULT([yes])
						CFLAGS="$OLD_CFLAGS -Wno-$1"
						# Optionally, run a test
						if test "x$2" != "x"; then
							AC_MSG_CHECKING([whether $CC can actually use -Wno-$1])
							CFLAGS="$OLD_CFLAGS -Werror -W$1"
							AC_COMPILE_IFELSE(
								[AC_LANG_SOURCE([$2])],
								[
									AC_MSG_RESULT([not needed but enabled])
									CFLAGS="$OLD_CFLAGS"
								],
								[
									CFLAGS="$OLD_CFLAGS -Werror -Wno-$1"
									AC_COMPILE_IFELSE(
										[AC_LANG_SOURCE([$2])],
										[
											AC_MSG_RESULT([yes])
											CFLAGS="$OLD_CFLAGS -Wno-$1"
										],
										[
											AC_MSG_RESULT([no])
											CFLAGS="$OLD_CFLAGS"
										]
									)
								]
							)
						fi
					],
					[
						AC_MSG_RESULT([no])
						CFLAGS="$OLD_CFLAGS"
					]
				)
			],
			[
				AC_MSG_RESULT([no])
				CFLAGS="$OLD_CFLAGS"
			]
		)
	]
)

AC_CHECK_COMPILER_WNOFLAG(unused-parameter, [int foo(int bar) { return 0; }])
AC_CHECK_COMPILER_WFLAG(empty-body)
AC_CHECK_COMPILER_WNOFLAG(switch)
AC_CHECK_COMPILER_WNOFLAG(missing-field-initializers)
AC_MSG_CHECKING([whether compiler is clang])
if test -n "`$CC --version | grep -i clang`" ; then
	AC_MSG_RESULT([yes, skipping some flags..])
else
	AC_MSG_RESULT([no])
	AC_CHECK_COMPILER_WNOFLAG(maybe-uninitialized)
	AC_CHECK_COMPILER_WNOFLAG(clobbered)
fi

# Certain versions of gcc make -Wshadow completely useless by making it flood
# you with unnecessary warnings <https://lkml.org/lkml/2006/11/28/239>
# Let's check if we can really use it
SAVED_OLD_CFLAGS="$CFLAGS"
AC_CHECK_COMPILER_WFLAG(shadow)
if test "x$CFLAGS" != "x$SAVED_OLD_CFLAGS"; then
	AC_MSG_CHECKING([whether $CC can efficiently use -Wshadow])
	NEW_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -Werror -Wshadow"
	AC_COMPILE_IFELSE(
		[AC_LANG_SOURCE([
			int foo(void) {
				return 0;
			}
			int bar(void) {
				int foo = 0;
				return foo + 1;
			}
		])],
		[
			AC_MSG_RESULT([yes])
			CFLAGS="$NEW_CFLAGS"
		],
		[
			AC_MSG_RESULT([no])
			CFLAGS="$SAVED_OLD_CFLAGS"
		]
	)
fi

AC_MSG_CHECKING([whether $CC supports -fPIC])
OLD_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -fPIC"
AC_COMPILE_IFELSE(
	[AC_LANG_SOURCE([int foo;])],
	[
		AC_MSG_RESULT([yes])
		compiler_supports_pic="yes"
	],
	[
		AC_MSG_RESULT([no])
		compiler_supports_pic="no"
	]
)
CFLAGS="$OLD_CFLAGS"

#
# option for shared objects
#
AC_MSG_CHECKING([how to make shared objects])
OLD_CFLAGS="$CFLAGS"
compiler_shared_objects=""
compiler_supports_shared_objects="no"
if test "$compiler_supports_pic" = "yes" ; then
	my_shared_test_flags="$CFLAGS -fPIC"
fi
# default
CFLAGS="$my_shared_test_flags -shared"
AC_LINK_IFELSE(
	[AC_LANG_SOURCE([
		int bar = 0;

		int foo(void)
		{
			return bar;
		}
	])],
	[
		compiler_shared_objects="-shared"
		compiler_supports_shared_objects="yes"
	]
)
# BeOS specific
CFLAGS="$my_shared_test_flags -nostart"
AC_LINK_IFELSE(
	[AC_LANG_SOURCE([
		int bar = 0;

		int foo(void)
		{
			return bar;
		}
	])],
	[
		compiler_shared_objects="-nostart"
		compiler_supports_shared_objects="yes"
	]
)
CFLAGS="$OLD_CFLAGS"
if test "$compiler_supports_shared_objects" = "no" ; then
	AC_MSG_RESULT([not supported])
	AC_MSG_NOTICE([compiler is unable to generate shared objects, disabled plugins (optional)])
	WITH_PLUGINS="no"
else
	AC_MSG_RESULT([$compiler_shared_objects $compiler_supports_shared_objects])
	SOFLAGS="$SOFLAGS $compiler_shared_objects"
	AC_SUBST([SOFLAGS])

	#
	# On certain platforms, undefined references on shared libraries won't be checked
	# unless explicitly required with the --no-undefined linker option
	#
	AC_MSG_CHECKING([whether $CC needs -Wl,--no-undefined to check for undefined references in shared objects])
	OLD_CFLAGS="$CFLAGS"
	CFLAGS="$SOFLAGS"
	AC_LINK_IFELSE(
		[AC_LANG_SOURCE([
			void foo(void) {
				foobar();
			}
		])],
		[
			CFLAGS="$SOFLAGS -Wl,--no-undefined"
			AC_LINK_IFELSE(
				[AC_LANG_SOURCE([
					int bar = 0;

					int foo(void) {
						return bar;
					}
				])],
				[
					AC_MSG_RESULT([yes])
					SOFLAGS="$SOFLAGS -Wl,--no-undefined"
				],
				[
					AC_MSG_RESULT([unsupported (undefined references check will be ignored)])
				]
			)
		],
		[
			AC_MSG_RESULT([no])
		]
	)
	CFLAGS="$OLD_CFLAGS"

	#
	# shared objects need position independent code; some platforms emit
	# it always, others need -fPIC
	#
	AC_MSG_CHECKING([whether $CC needs -fPIC for shared objects])
	OLD_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS $SOFLAGS"
	WITH_PLUGINS="yes"
	AC_LINK_IFELSE(
		[AC_LANG_SOURCE([
			int bar = 0;

			int foo(void)
			{
				return bar;
			}
		])],
		[
			AC_MSG_RESULT([no])
			CFLAGS="$OLD_CFLAGS"
		],
		[
			if test "$compiler_supports_pic" = "yes" ; then
				# Verify if -shared really fails due to lack of -fPIC or something else
				CFLAGS="$CFLAGS -fPIC"
				AC_LINK_IFELSE(
					[AC_LANG_SOURCE([
						int bar = 0;

						int foo(void)
						{
							return bar;
						}
					])],
					[
						AC_MSG_RESULT([yes])
						CFLAGS="$OLD_CFLAGS -fPIC"
					],
					[
						AC_MSG_RESULT([no, but fails for another reason])
						AC_MSG_ERROR([compiler is unable to compile shared objects for an unhandled reason, please report this with attached config.log... stopping])
					]
				)
			else
				# Disable compilation of plugins (optional), so 'make all' does not fail
				AC_MSG_RESULT([yes, but unsupported])
				AC_MSG_NOTICE([compiler is unable to generate position independent code, disabled plugins (optional)])
				WITH_PLUGINS="no"
			fi
		]
	)

fi
my_shared_test_flags=""
AC_SUBST([WITH_PLUGINS])


#
# -O2 implies -fstrict-aliasing, but the code is not safe for that
#
AC_MSG_CHECKING([whether $CC supports -fno-strict-aliasing])
OLD_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS -fno-strict-aliasing"
AC_COMPILE_IFELSE(
	[AC_LANG_SOURCE([int foo;])],
	[AC_MSG_RESULT([yes])],
	[
		AC_MSG_RESULT([no])
		CPPFLAGS="$OLD_CPPFLAGS"
	]
)


###############################################################################
# Check for libraries and header files.
#

#
# getaddrinfo - used to retrive ips from name
#
AC_CHECK_FUNC([getaddrinfo],[CPPFLAGS="$CPPFLAGS -DHAVE_GETADDRINFO"])

#
# setrlimit - used to set the socket limit
#
AC_CHECK_FUNC([setrlimit],[CPPFLAGS="$CPPFLAGS -DHAVE_SETRLIMIT"])


#
# strnlen - string length with upper scan bound
#
AC_CHECK_FUNC([strnlen],[CPPFLAGS="$CPPFLAGS -DHAVE_STRNLEN"])

# libconfig
AC_CHECK_FUNCS([uselocale])
AC_CHECK_FUNCS([newlocale])
AC_CHECK_FUNCS([freelocale])
AC_CHECK_HEADERS([xlocale.h], [], [], [
AC_LANG_SOURCE([#ifdef HAVE_XLOCALE_H
     # include <xlocale.h>
     #endif
     ])])

#
# Memory manager
#
case $enable_manager in
	"no")
		CPPFLAGS="$CPPFLAGS -DNO_MEMMGR"
		;;
	"builtin")
		# enabled by default
		;;
	"memwatch")
		CPPFLAGS="$CPPFLAGS -DMEMWATCH"
		AC_CHECK_HEADER([memwatch.h], , [AC_MSG_ERROR([memwatch header not found... stopping])])
		;;
	"dmalloc")
		CPPFLAGS="$CPPFLAGS -DDMALLOC -DDMALLOC_FUNC_CHECK"
		LIBS="$LIBS -ldmalloc"
		AC_CHECK_HEADER([dmalloc.h], , [AC_MSG_ERROR([dmalloc header not found... stopping])])
		;;
	"gcollect")
		CPPFLAGS="$CPPFLAGS -DGCOLLECT"
		LIBS="$LIBS -lgc"
		AC_CHECK_HEADER([gc.h], , [AC_MSG_ERROR([gcollect header not found... stopping])])
		;;
	"bcheck")
		CPPFLAGS="$CPPFLAGS -DBCHECK"
		;;
esac


#
# Packetver
#
if test -n "$enable_packetver" ; then
	CPPFLAGS="$CPPFLAGS -DPACKETVER=$enable_packetver"
fi


#
# Epoll
#
case $have_linux_epoll in
	"yes")
		CPPFLAGS="$CPPFLAGS -DSOCKET_EPOLL"
		;;
	"no")
		# default value
		;;
esac


#
# Debug
#
case $enable_debug in
	"no")
		# default value
		CPPFLAGS="$CPPFLAGS"
		;;
	"yes")
		if test $ac_cv_prog_cc_g = yes; then
		    CPPFLAGS="$CPPFLAGS -g -DDEBUG"
		    LDFLAGS="$LDFLAGS -g"
                else
		    echo "error: --enable-debug flag=$enable_debug entered but the compiler does not support -g"
		    exit 1
                fi
		;;
	"gdb")
		if test $ac_cv_prog_cc_g = yes; then
		    CPPFLAGS="$CPPFLAGS -ggdb -DDEBUG"
		    LDFLAGS="$LDFLAGS -g"
		else
		    echo "error: --enable-debug flag=$enable_debug entered but the compiler does not support -g"
		    exit 1
                fi
		;;
esac

#
# Renewal
#
case $enable_prere in
	"no")
		# default value
		CPPFLAGS="$CPPFLAGS"
		;;
	"yes")
		CPPFLAGS="$CPPFLAGS -DPRERE"
		;;
esac

#
# VIP
#
case $enable_vip in
	"no")
		# default value
		CPPFLAGS="$CPPFLAGS"
		;;
	"yes")
		CPPFLAGS="$CPPFLAGS -DVIP_ENABLE"
		;;
esac

#
# Warnings
#
case $enable_warn in
	"no") #default
		CPPFLAGS="$CPPFLAGS $PROD_WARN"
		;;
	"yes")
		CPPFLAGS="$CPPFLAGS"
		;;
	"extra")
		CPPFLAGS="$CPPFLAGS $EXTRA_WARN"
		;;
esac

#
# Buildbot
#
case $enable_buildbot in
	"no")
		# default value
		;;
	"yes")
		CPPFLAGS="$CPPFLAGS -DBUILDBOT"
		;;
esac

#
# RDTSC
#
case $enable_rdtsc in
	0)
		#default value
		;;
	1)
		CPPFLAGS="$CPPFLAGS -DENABLE_RDTSC"
		;;
esac


#
# Profiler
#
case $enable_profiler in
	"no")
		# default value
		;;
	"gprof")
		CFLAGS="$CFLAGS -pg"
		LDFLAGS="$LDFLAGS -pg"
		;;
esac


#
# zlib library (required)
#
if test -n "${ZLIB_HOME}" ; then
	LDFLAGS="$LDFLAGS -L${ZLIB_HOME}/lib"
	CPPFLAGS="$CPPFLAGS -I${ZLIB_HOME}/include"
fi
AC_SEARCH_LIBS([inflateEnd], [z], [],[AC_MSG_ERROR([zlib library not found or incompatible, please specify the correct path with --with-zlib=DIR... stopping])])
AC_CHECK_HEADER([zlib.h], [], [AC_MSG_ERROR([zlib header not found, please specify the correct path with --with-zlib=DIR... stopping])],
[AC_LANG_SOURCE([#ifdef HAVE_ZLIB_H
     # include <zlib.h>
     #endif
 ])])


#
# math library (required)
#
AC_SEARCH_LIBS([sqrt], [m], [], [AC_MSG_ERROR([math library not found... stopping])])


#
# clock_gettime (optional, rt on Debian)
#
AC_SEARCH_LIBS([clock_gettime], [rt])


#
# CLOCK_MONOTONIC clock for clock_gettime
# Normally defines _POSIX_TIMERS > 0 and _POSIX_MONOTONIC_CLOCK (for posix
# compliant systems) and __FreeBSD_cc_version >= 500005 (for FreeBSD
# >= 5.1.0, which does not have the posix defines (ref. r11983)) would be
# checked but some systems define them even when they do not support it
# (ref. bugreport:1003).
#
if test "$ac_cv_search_clock_gettime" != "no" ; then
	AC_MSG_CHECKING([whether CLOCK_MONOTONIC is supported and works])
	AC_RUN_IFELSE(
		[AC_LANG_SOURCE([
			#include <sys/time.h>
			#include <time.h>
			#include <unistd.h>
			
			int main(int argc, char** argv)
			{
				struct timespec tval;
				return clock_gettime(CLOCK_MONOTONIC, &tval);
			}
		])],
		[
			AC_MSG_RESULT([yes])
			CPPFLAGS="$CPPFLAGS -DHAVE_MONOTONIC_CLOCK"
		],
		[
			# either it failed to compile (CLOCK_MONOTONIC undefined)
			# or clock_gettime has returned a non-zero value
			AC_MSG_RESULT([no])
		],
		[
			AC_MSG_RESULT([guessing no])
		]
	)
fi


#
# pthread
#
AC_SEARCH_LIBS([pthread_create], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
AC_SEARCH_LIBS([pthread_sigmask], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
AC_SEARCH_LIBS([pthread_attr_init], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
AC_SEARCH_LIBS([pthread_attr_setstacksize], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
AC_SEARCH_LIBS([pthread_attr_destroy], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
AC_SEARCH_LIBS([pthread_cancel], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
AC_SEARCH_LIBS([pthread_join], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])


#
# MySQL library
#

if test -z "$MYSQL_CONFIG_HOME"; then
	AC_PATH_PROG([MYSQL_CONFIG_HOME], [mysql_config], [no])
fi

if test "$MYSQL_CONFIG_HOME" != "no" ; then
	MYSQL_VERSION="`$MYSQL_CONFIG_HOME --version`"
	if test "$manual_MYSQL_CFLAGS" = "no" ; then
		MYSQL_CFLAGS="`$MYSQL_CONFIG_HOME --include`"
	fi
	if test "$manual_MYSQL_LIBS" = "no" ; then
		MYSQL_LIBS="`$MYSQL_CONFIG_HOME --libs`"
	fi
else
	MYSQL_VERSION="unknown"
fi

MYSQL_OLD_LDFLAGS="$LDFLAGS" ; LDFLAGS="$LDFLAGS $MYSQL_LIBS"
MYSQL_OLD_CPPFLAGS="$CPPFLAGS" ; CPPFLAGS="$CPPFLAGS $MYSQL_CFLAGS"
AC_CHECK_LIB([mysqlclient], [mysql_init], [HAVE_MYSQL="yes"], [])
AC_CHECK_HEADER([mysql.h], [], [HAVE_MYSQL=""])
CPPFLAGS="$MYSQL_OLD_CPPFLAGS"
LDFLAGS="$MYSQL_OLD_LDFLAGS"

AC_MSG_CHECKING([MySQL library (required)])
if test "$HAVE_MYSQL" = "yes" ; then
	AC_MSG_RESULT([yes ($MYSQL_VERSION)])
else
	AC_MSG_RESULT([no])
	AC_MSG_ERROR([MySQL not found or incompatible])
fi

AC_SUBST([HAVE_MYSQL])
AC_SUBST([MYSQL_VERSION])
AC_SUBST([MYSQL_CFLAGS])
AC_SUBST([MYSQL_LIBS])


#
# PCRE library (optional)
#
PCRE_LIBS=""
PCRE_CFLAGS=""

if test "$want_pcre" = "no" ; then
	AC_MSG_NOTICE([ignoring PCRE (optional)])
else
	host_os="`uname`"
	if test "$host_os" = "FreeBSD" ; then
		if test -z "$PCRE_HOME" ; then PCRE_HOME="/usr/local"; fi
	fi
	if test -z "$PCRE_HOME" ; then
                AC_SEARCH_LIBS([pcre_study], [pcre], [HAVE_PCRE="yes"], [])
		if test "$HAVE_PCRE" = "yes" ; then
			PCRE_LIBS="-lpcre"
		fi
	else
		PCRE_OLD_LDFLAGS="$LDFLAGS" ; LDFLAGS="$LDFLAGS -L$PCRE_HOME/lib"
		PCRE_OLD_CPPFLAGS="$CPPFLAGS" ; CPPFLAGS="$CPPFLAGS -I$PCRE_HOME/include"
		AC_CHECK_LIB(pcre, pcre_compile, [HAVE_PCRE="yes"], [])
		CPPFLAGS="$PCRE_OLD_CPPFLAGS"
		LDFLAGS="$PCRE_OLD_LDFLAGS"
		if test "$HAVE_PCRE" = "yes" ; then
			PCRE_LIBS="-L$PCRE_HOME/lib -lpcre"
			test -d "$PCRE_HOME/include" && PCRE_CFLAGS="-I$PCRE_HOME/include"
		fi
	fi
	AC_MSG_CHECKING([PCRE library (optional)])
	if test "$HAVE_PCRE" = "yes" ; then
		PCRE_VERSION="`pcre-config --version`"
		AC_MSG_RESULT([yes ($PCRE_VERSION)])
	else
		AC_MSG_RESULT([no])
		if test "$require_pcre" = "yes" ; then
			AC_MSG_ERROR([PCRE not found or incompatible (requested)])
		else
			AC_MSG_NOTICE([disabling PCRE (optional)])
		fi
	fi
fi

AC_SUBST([HAVE_PCRE])
AC_SUBST([PCRE_VERSION])
AC_SUBST([PCRE_LIBS])
AC_SUBST([PCRE_CFLAGS])

#
# Host specific stuff
#
AC_MSG_CHECKING([host OS])
host_os="`uname`"
AC_MSG_RESULT([$host_os])
fd_setsize=""
DLLEXT=".so"
case $host_os in
Solaris* )
	LIBS="$LIBS -lsocket -lnsl -ldl"
	;;
Linux* )
	LIBS="$LIBS -ldl"
	;;
FreeBSD*)
	CPPFLAGS="$CPPFLAGS -D__FREEBSD__"
	;;
NetBSD*)
	CPPFLAGS="$CPPFLAGS -D__NETBSD__"
	;;
CYGWIN*)
	CPPFLAGS="$CPPFLAGS -DFD_SETSIZE=4096 -DCYGWIN"
	fd_setsize="done"
	DLLEXT=".dll"
	;;
esac
AC_SUBST([DLLEXT])

AC_MSG_CHECKING([for MinGW])
if test -n "`$CC --version | grep -i mingw`" ; then
	AC_MSG_RESULT([yes])
	CPPFLAGS="$CPPFLAGS -DMINGW"
	if test -z "$fd_setsize" ; then
		CPPFLAGS="$CPPFLAGS -DFD_SETSIZE=4096"
	fi
	LIBS="$LIBS -lws2_32"
else
	AC_MSG_RESULT([no])
fi

CXXFLAG_CLEARS="-std=c++17 $CPPFLAGS"
CFLAGS="$OPT_LTO $CFLAGS" 
CFLAGS_AR="$OPT_LTO_AR $CFLAGS"
AC_SUBST([CFLAGS_AR])
AC_SUBST([CXXFLAG_CLEARS])

AC_MSG_NOTICE([Configure finish])
AC_MSG_NOTICE([CPPFLAGS= $CPPFLAGS])
AC_MSG_NOTICE([CFLAGS= $CFLAGS])
AC_MSG_NOTICE([CFLAGS_AR= $CFLAGS_AR])
AC_MSG_NOTICE([LDFLAGS= $LDFLAGS])
#AC_MSG_NOTICE([PROD_WARN= $PROD_WARN])
#AC_MSG_NOTICE([EXTRA_WARN= $EXTRA_WARN])
AC_MSG_NOTICE([output name =  $output_login, $output_char, $output_map, $output_web])

###############################################################################
AC_OUTPUT
