all: login-converter
sql: login-converter

COMMON_OBJ = ../../common/core.o ../../common/socket.o ../../common/timer.o ../../common/db.o ../../common/malloc.o ../../common/showmsg.o
COMMON_H = ../../common/core.h ../../common/socket.h ../../common/timer.h ../../common/mmo.h ../../common/version.h ../../common/db.h ../../common/malloc.h

login-converter: login-converter.o ../../login_sql/md5calc.o ../../login_sql/strlib.o $(COMMON_OBJ)
	$(CC) -o ../../../$@ $^ $(LIB_S)
login-converter.o: login-converter.c ../../login_sql/login.h ../../login_sql/md5calc.h ../../login_sql/strlib.h $(COMMON_H)
clean:
	rm -f *.o ../../../login-converter
