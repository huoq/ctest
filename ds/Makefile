#include dsa/source/Makefile ds/comm/src/Makefile

dsclient : dsclient.o dsa/source/*.o ds/comm/src/*.o
	gcc -g -o dsclient dsclient.o dsa/source/*.o ds/comm/src/*.o -lpthread -lelf -ldl
dsclient.o : dsclient.c
	gcc -g -c dsclient.c -Ids/comm/inc

clean :
	#rm *.o dsclient dsa/source/*.o ds/comm/src/*.o
	rm *.o dsclient
