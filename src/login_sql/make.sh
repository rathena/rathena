#!/bin/sh
	rsqlt=`rm -rf *.o`
	gcc -c login.c -I/usr/local/include/mysql/
	gcc -c md5calc.c -I/usr/local/include/mysql/
	gcc -c strlib.c
	gcc -o login-server login.o strlib.o md5calc.o ../common/core.o ../common/socket.o ../common/timer.o ../common/db.o -L/usr/local/lib/mysql/ -lmysqlclient -lz
