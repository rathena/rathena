all: login-converter
sql: login-converter

COMMON_OBJ = ../../common/core.o ../../common/socket.o ../../common/timer.o ../../common/grfio.o ../../common/db.o ../../common/lock.o ../../common/nullpo.o ../../common/malloc.o ../../common/showmsg.o ../../common/strlib.o

COMMON_H = ../../common/core.h ../../common/socket.h ../../common/timer.h ../../common/mmo.h ../../common/version.h ../../common/db.h ../../common/malloc.h ../../common/strlib.h

login-converter: login-converter.o ../../login_sql/md5calc.o $(COMMON_OBJ)
	$(CC) -o ../../../$@ $^ $(LIB_S)
login-converter.o: login-converter.c ../../login_sql/login.h ../../login_sql/md5calc.h $(COMMON_H)
clean:
	rm -f *.o ../../../login-converter
