all: char-converter
sql: char-converter

COMMON_OBJ = ../../common/core.o ../../common/socket.o ../../common/timer.o ../../common/grfio.o ../../common/db.o ../../common/lock.o ../../common/nullpo.o ../../common/malloc.o ../../common/showmsg.o ../../common/strlib.o

char-converter: char-converter.o $(COMMON_OBJ)
	$(CC) -o ../../../$@ $^ $(LIB_S)

char-converter.o: char-converter.c char.h ../../common/strlib.h
strlib.o: strlib.c ../../common/strlib.h
clean:
	rm -f *.o ../../../char-converter

