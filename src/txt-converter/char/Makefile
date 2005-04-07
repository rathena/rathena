all: char-converter
sql: char-converter

COMMON_OBJ = ../../common/obj/core.o ../../common/obj/socket.o ../../common/obj/timer.o ../../common/obj/grfio.o ../../common/obj/db.o ../../common/obj/lock.o ../../common/obj/nullpo.o ../../common/obj/malloc.o ../../common/obj/showmsg.o ../../common/obj/strlib.o

char-converter: char-converter.o $(COMMON_OBJ)
	$(CC) -o ../../../$@ $^ $(LIB_S)

char-converter.o: char-converter.c char.h ../../common/strlib.h
strlib.o: strlib.c ../../common/strlib.h
clean:
	rm -f *.o ../../../char-converter

