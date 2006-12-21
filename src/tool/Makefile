all: adduser convert

adduser:
	$(CC) -o ../../tools/$@ adduser.c

convert:
	$(CC) -o ../../tools/$@ convert.c

clean:
	rm -rf ../../tools/adduser ../../tools/convert
