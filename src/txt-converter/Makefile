all sql: char-converter login-converter

char-converter: char-converter.o ../common/obj/minicore.o \
	../common/obj/malloc.o ../common/obj/showmsg.o ../common/obj/strlib.o \
	../common/obj/mapindex.o ../common/obj/ers.o ../common/obj/utils.o \
	../char/char.o ../char/int_pet.o ../char/int_storage.o ../char/inter.o \
	../char/int_party.o ../char/int_guild.o \
	../char_sql/char.o ../char_sql/int_pet.o ../char_sql/int_storage.o \
	../char_sql/inter.o ../char_sql/int_party.o ../char_sql/int_guild.o
	$(CC) -o ../../tools/$@ $^ $(LIB_S)

login-converter: login-converter.o ../common/obj/minicore.o ../common/obj/db.o ../common/obj/malloc.o ../common/obj/showmsg.o ../common/obj/ers.o ../common/obj/utils.o
	$(CC) -o ../../tools/$@ $^ $(LIB_S)

clean:
	rm -f *.o ../../tools/login-converter ../../tools/char-converter GNUmakefile

# DO NOT DELETE

%.o: %.c
	$(COMPILE.c) -DTXT_SQL_CONVERT $(OUTPUT_OPTION) $<

char-converter.o: char-converter.c
login-converter.o: login-converter.c
