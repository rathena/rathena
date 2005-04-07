txt sql all: obj common

obj:
	mkdir obj

common: obj/core.o obj/socket.o obj/timer.o obj/grfio.o obj/db.o obj/lock.o obj/nullpo.o obj/malloc.o obj/showmsg.o  obj/strlib.o obj/utils.o

obj/%.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

obj/core.o: core.c core.h showmsg.h
obj/socket.o: socket.c socket.h mmo.h showmsg.h dll.h
obj/timer.o: timer.c timer.h showmsg.h
obj/grfio.o: grfio.c grfio.h showmsg.h
obj/db.o: db.c db.h showmsg.h
obj/lock.o: lock.h showmsg.h
obj/nullpo.o: nullpo.c nullpo.h showmsg.h
obj/malloc.o: malloc.c malloc.h showmsg.h
obj/showmsg.o: showmsg.c showmsg.h
obj/strlib.o: strlib.c strlib.h utils.h

clean:
	rm -rf *.o obj
