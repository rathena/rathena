all: ladmin
txt: ladmin
sql: ladmin

COMMON_OBJ = ../common/obj/core.o ../common/obj/socket.o ../common/obj/timer.o ../common/obj/db.o ../common/obj/malloc.o ../common/obj/showmsg.o ../common/obj/strlib.o
COMMON_H = ../common/core.h ../common/socket.h ../common/timer.h ../common/mmo.h ../common/version.h ../common/db.h ../common/malloc.h ../common/showmsg.h ../common/strlib.h

ladmin: ladmin.o md5calc.o $(COMMON_OBJ)
	$(CC) -o ../../$@ ladmin.o md5calc.o $(COMMON_OBJ) $(LIB_S)
ladmin.o: ladmin.c ladmin.h md5calc.h $(COMMON_H)
md5calc.o: md5calc.c md5calc.h

clean:
	rm -f *.o ../../ladmin
