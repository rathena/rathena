all:
	#Generate framework...
	$(CC) -c parse.c
	$(CC) -c generate.c
	$(CC) -c htmlstyle.c
	$(CC) -c logs.c
	
	#Generate "pages"...
	cd pages && $(CC) -c about.c && cd ..
	cd pages && $(CC) -c sample.c && cd ..
	cd pages && $(CC) -c notdone.c && cd ..

	#Building the server...
	$(CC) -o webserver main.c parse.o generate.o htmlstyle.o \
	logs.o pages/about.o pages/sample.o pages/notdone.o

clean:
	rm -f *.o
	rm -f pages/*.o
	rm -f webserver
