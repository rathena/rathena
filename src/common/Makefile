txt sql all: obj common

obj:
	mkdir obj

common: obj/core.o obj/socket.o obj/timer.o obj/db.o obj/plugins.o obj/lock.o \
	obj/nullpo.o obj/malloc.o obj/showmsg.o  obj/strlib.o obj/utils.o \
	obj/graph.o obj/grfio.o obj/minicore.o obj/minisocket.o obj/minimalloc.o \
	obj/mapindex.o obj/unz.o obj/ers.o


obj/%.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

obj/mini%.o: %.c
	$(COMPILE.c) -DMINICORE $(OUTPUT_OPTION) $<

obj/unz.o:
	$(MAKE) -C ../zlib
	@touch $@


clean:
	rm -rf *.o obj

HAVESVN = $(shell which svnversion)

ifeq ($(findstring /,$(HAVESVN)), /)
svnversion.h:
	@printf "#define SVNVERSION " > svnversion.h
	@svnversion . >> svnversion.h
else
svnversion.h:
	@printf "" > svnversion.h
endif

obj/minicore.o: core.c core.h
obj/minisocket.o: socket.c socket.h
obj/minimalloc.o: malloc.c malloc.h

# DO NOT DELETE

obj/core.o: core.c core.h showmsg.h svnversion.h
obj/socket.o: socket.c socket.h mmo.h showmsg.h plugins.h
obj/timer.o: timer.c timer.h showmsg.h
obj/ers.o: ers.c ers.h cbasetypes.h
obj/db.o: db.c db.h showmsg.h ers.h
obj/lock.o: lock.c lock.h showmsg.h
obj/grfio.o: grfio.c grfio.h
obj/graph.o: graph.c graph.h
obj/nullpo.o: nullpo.c nullpo.h showmsg.h
obj/malloc.o: malloc.c malloc.h showmsg.h
obj/plugins.o: plugins.c plugins.h plugin.h
obj/showmsg.o: showmsg.c showmsg.h
obj/strlib.o: strlib.c strlib.h utils.h
obj/mapindex.o: mapindex.c mapindex.h
