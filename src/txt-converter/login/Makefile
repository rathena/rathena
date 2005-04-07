all: login-converter
sql: login-converter

COMMON_OBJ = ../../common/obj/core.o ../../common/obj/socket.o ../../common/obj/timer.o ../../common/obj/grfio.o ../../common/obj/db.o ../../common/obj/lock.o ../../common/obj/nullpo.o ../../common/obj/malloc.o ../../common/obj/showmsg.o ../../common/obj/strlib.o

COMMON_H = ../../common/core.h ../../common/socket.h ../../common/timer.h ../../common/mmo.h ../../common/version.h ../../common/db.h ../../common/malloc.h ../../common/strlib.h

login-converter: login-converter.o ../../login_sql/md5calc.o $(COMMON_OBJ)
	$(CC) -o ../../../$@ $^ $(LIB_S)
login-converter.o: login-converter.c ../../login_sql/login.h ../../login_sql/md5calc.h $(COMMON_H)
clean:
	rm -f *.o ../../../login-converter
