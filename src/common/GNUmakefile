txt sql all: core.o socket.o timer.o grfio.o db.o lock.o nullpo.o malloc.o showmsg.o

core.o: core.c core.h showmsg.h
socket.o: socket.c socket.h mmo.h showmsg.h
timer.o: timer.c timer.h showmsg.h
grfio.o: grfio.c grfio.h showmsg.h
db.o: db.c db.h showmsg.h
lock.o: lock.h showmsg.h
nullpo.o: nullpo.c nullpo.h showmsg.h
malloc.o: malloc.c malloc.h showmsg.h
showmsg.o: showmsg.c showmsg.h

clean:
	rm -f *.o
