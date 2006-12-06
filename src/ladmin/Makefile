all txt sql: ladmin

COMMON_OBJ = ../common/obj/minicore.o ../common/obj/minisocket.o ../common/obj/timer.o \
	../common/obj/malloc.o ../common/obj/showmsg.o ../common/obj/strlib.o \
	../common/obj/utils.o
COMMON_H = ../common/core.h ../common/socket.h ../common/timer.h ../common/mmo.h \
	../common/version.h ../common/malloc.h ../common/showmsg.h ../common/strlib.h \
	../common/utils.h

ladmin: ladmin.o md5calc.o $(COMMON_OBJ)
	$(CC) -o ../../$@ ladmin.o md5calc.o $(COMMON_OBJ) $(LIB_S)

clean:
	rm -f *.o ../../ladmin

# DO NOT DELETE

ladmin.o: ladmin.c ladmin.h md5calc.h $(COMMON_H)
md5calc.o: md5calc.c md5calc.h
