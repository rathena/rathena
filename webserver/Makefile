cc = gcc

all:
	#Generate framework...
	$(cc) -c parse.c
	$(cc) -c generate.c
	$(cc) -c htmlstyle.c
	$(cc) -c logs.c
	
	#Generate "pages"...
	cd pages && $(cc) -c about.c && cd ..
	cd pages && $(cc) -c sample.c && cd ..
	cd pages && $(cc) -c notdone.c && cd ..

	#Building the server...
	$(cc) -o webserver main.c parse.o generate.o htmlstyle.o \
	logs.o pages/about.o pages/sample.o pages/notdone.o
	



clean:
	rm -f *.o
	rm -f pages/*.o
	rm -f webserver
	
