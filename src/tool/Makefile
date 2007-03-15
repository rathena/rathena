all: adduser convert mapcache

adduser:
	$(CC) -o ../../tools/$@ adduser.c

convert:
	$(CC) -o ../../tools/$@ convert.c

mapcache:
	$(MAKE) -C ../zlib
	$(CC) -o ../../$@ mapcache.c ../zlib/unz.o

clean:
	rm -rf ../../tools/adduser ../../tools/convert ../../mapcache
