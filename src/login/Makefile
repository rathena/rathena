all txt: login-server

COMMON_OBJ = ../common/obj/core.o ../common/obj/socket.o ../common/obj/timer.o \
	../common/obj/db.o ../common/obj/plugins.o ../common/obj/lock.o \
	../common/obj/malloc.o ../common/obj/showmsg.o ../common/obj/utils.o \
	../common/obj/strlib.o ../common/obj/graph.o ../common/obj/grfio.o \
	../common/obj/mapindex.o ../common/obj/ers.o ../zlib/unz.o
COMMON_H = ../common/core.h ../common/socket.h ../common/timer.h ../common/mmo.h \
	../common/version.h ../common/db.h ../common/plugins.h ../common/lock.h \
	../common/malloc.h ../common/showmsg.h ../common/utils.h ../common/strlib.h \
	../common/graph.h ../common/grfio.h ../common/mapindex.h

%.o: %.c
	$(COMPILE.c) -DTXT_ONLY $(OUTPUT_OPTION) $<

login-server: login.o md5calc.o $(COMMON_OBJ)
	$(CC) -o ../../$@ login.o md5calc.o $(COMMON_OBJ) $(LIB_S)
	
clean:
	rm -f *.o ../../login-server

# DO NOT DELETE

login.o: login.c login.h md5calc.h $(COMMON_H)
md5calc.o: md5calc.c md5calc.h
