all: ladmin
txt: ladmin
sql: ladmin

COMMON_OBJ = ../common/core.o ../common/socket.o ../common/timer.o ../common/db.o ../common/malloc.o ../common/showmsg.o
COMMON_H = ../common/core.h ../common/socket.h ../common/timer.h ../common/mmo.h ../common/version.h ../common/db.h ../common/malloc.h ../common/showmsg.h

ladmin: ladmin.o md5calc.o $(COMMON_OBJ)
	$(CC) -o ../../$@ ladmin.o md5calc.o $(COMMON_OBJ)
ladmin.o: ladmin.c ladmin.h md5calc.h $(COMMON_H)
md5calc.o: md5calc.c md5calc.h

clean:
	rm -f *.o ../../ladmin
