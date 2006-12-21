
OBJS = unzip.o ioapi.o

ifeq ($(findstring MINGW,$(CFLAGS)), MINGW)
	OBJS += iowin32.o
endif

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all: unz.o

unz.o: $(OBJS)
	ar rcs $@ $(OBJS)

clean:
	rm -f *.o

# DO NOT DELETE
ioapi.o: ioapi.h
unzip.o: unzip.h ioapi.h crypt.h
