all: char-server_sql
sql: char-server_sql

COMMON_OBJ = ../common/core.o ../common/socket.o ../common/timer.o ../common/db.o ../common/malloc.o ../common/showmsg.o
COMMON_H = ../common/core.h ../common/socket.h ../common/timer.h ../common/db.h ../common/malloc.h ../common/showmsg.h

char-server_sql: char.o inter.o int_party.o int_guild.o int_storage.o int_pet.o strlib.o itemdb.o $(COMMON_OBJ)
	$(CC) -o ../../$@ $^ $(LIB_S)

char.o: char.c char.h strlib.h itemdb.h ../common/showmsg.h
inter.o: inter.c inter.h int_party.h int_guild.h int_storage.h int_pet.h ../common/mmo.h char.h ../common/socket.h ../common/showmsg.h
int_party.o: int_party.c int_party.h inter.h ../common/mmo.h char.h ../common/socket.h ../common/timer.h ../common/db.h ../common/showmsg.h
int_guild.o: int_guild.c int_guild.h inter.h ../common/mmo.h char.h ../common/socket.h ../common/db.h ../common/showmsg.h
int_storage.o: int_storage.c int_storage.h char.h itemdb.h ../common/showmsg.h
int_pet.o: int_pet.c int_pet.h inter.h char.h ../common/mmo.h ../common/socket.h ../common/db.h ../common/showmsg.h
strlib.o: strlib.c strlib.h ../common/showmsg.h
itemdb.o: itemdb.c itemdb.h ../common/db.h ../common/mmo.h ../common/showmsg.h

clean:
	rm -f *.o ../../char-server_sql
