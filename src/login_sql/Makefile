all: login-server_sql
sql: login-server_sql

shared_libs=all
COMMON_OBJ = ../common/core.o ../common/socket.o ../common/timer.o ../common/db.o ../common/malloc.o ../common/showmsg.o
COMMON_H = ../common/core.h ../common/socket.h ../common/timer.h ../common/mmo.h ../common/version.h ../common/db.h ../common/malloc.h ../common/showmsg.h

login-server_sql: login.o md5calc.o strlib.o $(COMMON_OBJ)
	$(CC) -o ../../$@ $^ $(LIB_S)

login.o: login.c login.h md5calc.h strlib.h $(COMMON_H)
md5calc.o: md5calc.c md5calc.h
strlib.o: strlib.c strlib.h

clean:
	rm -f *.o ../../login-server_sql

