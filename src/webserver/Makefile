CC = gcc -pipe
MAKE = make
OPT = -g -O2 -ffast-math

ifeq ($(findstring CYGWIN,$(PLATFORM)), CYGWIN)
CFLAGS = $(OPT) -Wall -I../common
else
CFLAGS = $(OPT) -Wall -I../common
endif

OBJ  = main.o webserver.o ../common/showmsg.o
LINKOBJ  = main.o webserver.o ../common/showmsg.o

all: clean webserver run

clean:
	rm -f *.o webserver.exe ../common/showmsg.o

webserver: main.o webserver.o ../common/showmsg.o
	$(CC) $(OPT) -o "Webserver.exe" $(OBJ)

main.o: main.c
	$(CC) $(OPT) -c main.c -o main.o $(CFLAGS)

webserver.o: webserver.c
	$(CC) $(OPT) -c webserver.c -o webserver.o $(CFLAGS)

../common/showmsg.o: ../common/showmsg.c
	$(CC) $(OPT) -c ../common/showmsg.c -o ../common/showmsg.o $(CFLAGS)

run:
	./webserver
