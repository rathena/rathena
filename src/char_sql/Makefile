all: char-server_sql
sql: char-server_sql

COMMON_OBJ = ../common/obj/core.o ../common/obj/socket.o ../common/obj/timer.o ../common/obj/db.o ../common/obj/malloc.o ../common/obj/showmsg.o ../common/obj/utils.o ../common/obj/strlib.o
COMMON_H = ../common/core.h ../common/socket.h ../common/timer.h ../common/db.h ../common/malloc.h ../common/showmsg.h ../common/utils.h ../common/strlib.h

char-server_sql: char.o inter.o int_party.o int_guild.o int_storage.o int_pet.o itemdb.o $(COMMON_OBJ)
	$(CC) -o ../../$@ $^ $(LIB_S)

char.o: char.c char.h ../common/strlib.h itemdb.h ../common/showmsg.h
inter.o: inter.c inter.h int_party.h int_guild.h int_storage.h int_pet.h ../common/mmo.h char.h ../common/socket.h ../common/showmsg.h
int_party.o: int_party.c int_party.h inter.h ../common/mmo.h char.h ../common/socket.h ../common/timer.h ../common/db.h ../common/showmsg.h
int_guild.o: int_guild.c int_guild.h inter.h ../common/mmo.h char.h ../common/socket.h ../common/db.h ../common/showmsg.h
int_storage.o: int_storage.c int_storage.h char.h itemdb.h ../common/showmsg.h
int_pet.o: int_pet.c int_pet.h inter.h char.h ../common/mmo.h ../common/socket.h ../common/db.h ../common/showmsg.h
itemdb.o: itemdb.c itemdb.h ../common/db.h ../common/mmo.h ../common/showmsg.h
$(COMMON_OBJ): $(COMMON_H)

clean:
	rm -f *.o ../../char-server_sql
