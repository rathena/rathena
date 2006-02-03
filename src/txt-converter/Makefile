all sql: char-converter login-converter

char-converter: char-converter.o ../common/obj/minicore.o ../common/obj/malloc.o ../common/obj/showmsg.o ../common/obj/strlib.o ../common/obj/mapindex.o ../common/obj/ers.o
	$(CC) -o ../../tools/$@ $^ $(LIB_S)

login-converter: login-converter.o ../common/obj/minicore.o ../common/obj/db.o ../common/obj/malloc.o ../common/obj/showmsg.o ../common/obj/ers.o
	$(CC) -o ../../tools/$@ $^ $(LIB_S)

clean:
	rm -f *.o ../../tools/login-converter ../../tools/char-converter

# DO NOT DELETE

char-converter.o: char-converter.c
login-converter.o: login-converter.c
