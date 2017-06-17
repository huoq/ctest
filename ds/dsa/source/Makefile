all : dsa_cmd.o dsa_init.o dsa_io.o dsa_msg.o dsa_parse_elf.o

dsa_cmd.o : dsa_cmd.c
	gcc -g -c dsa_cmd.c -I../include -I../../ds/comm/inc

dsa_init.o : dsa_init.c
	gcc -g -c dsa_init.c -I../include -I../../ds/comm/inc

dsa_io.o : dsa_io.c
	gcc -g -c dsa_io.c -I../include -I../../ds/comm/inc

dsa_msg.o: dsa_msg.c
	gcc -g -c dsa_msg.c -I../include -I../../ds/comm/inc

dsa_parse_elf.o : dsa_parse_elf.c
	gcc -g -c dsa_parse_elf.c -I../include -I../../ds/comm/inc

#clean : 
#	rm *.o
