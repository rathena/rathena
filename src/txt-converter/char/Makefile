all: char-converter
sql: char-converter

COMMON_OBJ = ../../common/core.o ../../common/socket.o ../../common/timer.o ../../common/db.o ../../common/malloc.o

char-converter: char-converter.o strlib.o $(COMMON_OBJ)
	$(CC) -o ../../../$@ $^ $(LIB_S)

char-converter.o: char-converter.c char.h strlib.h
strlib.o: strlib.c strlib.h
clean:
	rm -f *.o ../../../char-converter

