all: adduser convert mapcache

adduser:
	$(CC) -o ../../tools/$@ adduser.c

convert:
	$(CC) -o ../../tools/$@ convert.c

mapcache:
	$(CC) -c -o grfio.o grfio.c
	$(CC) -o ../../$@ mapcache.c grfio.o -lz

clean:
	rm -rf *.o ../../tools/adduser ../../tools/convert ../../mapcache
