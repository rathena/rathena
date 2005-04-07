
CC = gcc -pipe
# CC = gcc -pipe -DPCRE_SUPPORT
# CC = g++ --pipe
# CC = gcc -pipe -DGCOLLECT
# CC = gcc -pipe -DDMALLOC -DDMALLOC_FUNC_CHECK
# CC = /usr/local/bin/gcc -fbounds-checking -pipe -DBCHECK

# GCLIB = -lgc
# GCLIB = -L/usr/local/lib -lpcre
GCLIB =
# GCLIB = -ldmalloc

PACKETDEF = -DPACKETVER=6 -DNEW_006b -DSO_REUSEPORT
#PACKETDEF = -DPACKETVER=5 -DNEW_006b
#PACKETDEF = -DPACKETVER=4 -DNEW_006b
#PACKETDEF = -DPACKETVER=3 -DNEW_006b
#PACKETDEF = -DPACKETVER=2 -DNEW_006b
#PACKETDEF = -DPACKETVER=1 -DNEW_006b

PLATFORM = $(shell uname)

ifeq ($(findstring FreeBSD,$(PLATFORM)), FreeBSD)
MAKE = gmake
else
MAKE = make
endif
ifeq ($(findstring NetBSD,$(PLATFORM)), NetBSD)
MAKE = gmake
endif

OPT = -g -O2 -ffast-math -Wall -Wno-sign-compare
# OPT += -DDUMPSTACK -rdynamic

ifeq ($(findstring CYGWIN,$(PLATFORM)), CYGWIN)
OS_TYPE = -DCYGWIN
CFLAGS =  $(OPT) -DFD_SETSIZE=4096 -I../common $(PACKETDEF) $(OS_TYPE)
else
OS_TYPE =
CFLAGS =  $(OPT) -I../common $(PACKETDEF) $(OS_TYPE)
# CFLAGS = -DTWILIGHT  $(OPT) -Wall -I../common $(PACKETDEF) $(OS_TYPE)
endif

MYSQLFLAG_INCLUDE_DEFAULT = /usr/local/include/mysql

ifdef SQLFLAG
MYSQLFLAG_CONFIG = $(shell which mysql_config)
ifeq ($(findstring /,$(MYSQLFLAG_CONFIG)), /)
MYSQLFLAG_VERSION = $(shell $(MYSQLFLAG_CONFIG) --version | sed s:\\..*::) 
endif

ifeq ($(findstring 4,$(MYSQLFLAG_VERSION)), 4)
MYSQLFLAG_CONFIG_ARGUMENT = --cflags
endif
ifeq ($(findstring 5,$(MYSQLFLAG_VERSION)), 5)
MYSQLFLAG_CONFIG_ARGUMENT = --include
endif
ifndef MYSQLFLAG_CONFIG_ARGUMENT
MYSQLFLAG_CONFIG_ARGUMENT = --cflags
endif

ifeq ($(findstring /,$(MYSQLFLAG_CONFIG)), /)
MYSQLFLAG_INCLUDE = $(shell $(MYSQLFLAG_CONFIG) $(MYSQLFLAG_CONFIG_ARGUMENT))
else
MYSQLFLAG_INCLUDE = -I$(MYSQLFLAG_INCLUDE_DEFAULT)
endif

LIB_S_DEFAULT = -L/usr/local/lib/mysql -lmysqlclient -lz
MYSQLFLAG_CONFIG = $(shell which mysql_config)
ifeq ($(findstring /,$(MYSQLFLAG_CONFIG)), /)
LIB_S = $(shell $(MYSQLFLAG_CONFIG) --libs)
else
LIB_S = $(LIB_S_DEFAULT)
endif

MYLIB = CC="$(CC)" CFLAGS="$(CFLAGS) $(MYSQLFLAG_INCLUDE)" LIB_S="$(LIB_S) $(GCLIB)"

endif

MKDEF = CC="$(CC)" CFLAGS="$(CFLAGS)" LIB_S="$(GCLIB)"

all: conf txt

conf:
	cp -r conf-tmpl conf
	rm -rf conf/.svn conf/*/.svn
	cp -r save-tmpl save
	rm -rf save/.svn

txt : src/common/GNUmakefile src/login/GNUmakefile src/char/GNUmakefile src/map/GNUmakefile src/ladmin/GNUmakefile conf
	cd src ; cd common ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd login ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd char ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd map ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd ladmin ; $(MAKE) $(MKDEF) $@ ; cd ..


ifdef SQLFLAG
sql: src/common/GNUmakefile src/login_sql/GNUmakefile src/char_sql/GNUmakefile src/map/GNUmakefile src/txt-converter/login/GNUmakefile src/txt-converter/char/GNUmakefile conf
	cd src ; cd common ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd login_sql ; $(MAKE) $(MYLIB) $@ ; cd ..
	cd src ; cd char_sql ; $(MAKE) $(MYLIB) $@ ; cd ..
	cd src ; cd map ; $(MAKE) $(MYLIB) $@ ; cd ..
	cd src ; cd txt-converter ; cd login ; $(MAKE) $(MYLIB) ; cd ..
	cd src ; cd txt-converter ; cd char ; $(MAKE) $(MYLIB) ; cd ..
else
sql:
	$(MAKE) CC="$(CC)" OPT="$(OPT)" SQLFLAG=1 $@
endif


tools:
	cd src ; cd tool && $(MAKE) $(MKDEF) && cd ..

webserver:
	cd src ; cd webserver && $(MAKE) $(MKDEF) && cd ..


clean: src/common/GNUmakefile src/login/GNUmakefile src/char/GNUmakefile src/map/GNUmakefile src/ladmin/GNUmakefile src/txt-converter/login/GNUmakefile src/txt-converter/char/GNUmakefile
	cd src ; cd common ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd login ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd login_sql ; $(MAKE) $(MKLIB) $@ ; cd ..
	cd src ; cd char ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd char_sql ; $(MAKE) $(MKLIB) $@ ; cd ..
	cd src ; cd map ; $(MAKE) $(MKLIB) $@ ; cd ..
	cd src ; cd ladmin ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd txt-converter ; cd login ; $(MAKE) $(MKLIB) $@ ; cd ..
	cd src ; cd txt-converter ; cd char ; $(MAKE) $(MKLIB) $@ ; cd ..

src/common/GNUmakefile: src/common/Makefile
	sed -e 's/$$>/$$^/' src/common/Makefile > src/common/GNUmakefile
src/login/GNUmakefile: src/login/Makefile
	sed -e 's/$$>/$$^/' src/login/Makefile > src/login/GNUmakefile
src/login_sql/GNUmakefile: src/login_sql/Makefile
	sed -e 's/$$>/$$^/' src/login_sql/Makefile > src/login_sql/GNUmakefile
src/char/GNUmakefile: src/char/Makefile
	sed -e 's/$$>/$$^/' src/char/Makefile > src/char/GNUmakefile
src/char_sql/GNUmakefile: src/char_sql/Makefile
	sed -e 's/$$>/$$^/' src/char_sql/Makefile > src/char_sql/GNUmakefile
src/map/GNUmakefile: src/map/Makefile
	sed -e 's/$$>/$$^/' src/map/Makefile > src/map/GNUmakefile
src/ladmin/GNUmakefile: src/ladmin/Makefile
	sed -e 's/$$>/$$^/' src/ladmin/Makefile > src/ladmin/GNUmakefile
src/txt-converter/login/GNUmakefile: src/txt-converter/login/Makefile
	sed -e 's/$$>/$$^/' src/txt-converter/login/Makefile > src/txt-converter/login/GNUmakefile
src/txt-converter/char/GNUmakefile: src/txt-converter/char/Makefile
	sed -e 's/$$>/$$^/' src/txt-converter/char/Makefile > src/txt-converter/char/GNUmakefile
