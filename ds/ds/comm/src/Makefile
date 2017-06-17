all : ds_comm_cmd.o ds_comm_event.o ds_comm_if.o ds_comm_util.o ds_parse_lex.o ds_parse_yacc.o ds_symbol_mgr.o

ds_comm_cmd.o : ds_comm_cmd.c
	gcc -g -c ds_comm_cmd.c -I../inc -I../../../dsa/include

ds_comm_event.o : ds_comm_event.c
	gcc -g -c ds_comm_event.c -I../inc -I../../../dsa/include

ds_comm_if.o : ds_comm_if.c
	gcc -g -c ds_comm_if.c -I../inc -I../../../dsa/include

ds_comm_util.o : ds_comm_util.c
	gcc -g -c ds_comm_util.c -I../inc -I../../../dsa/include

ds_parse_lex.o : ds_parse_lex.c
	gcc -g -c ds_parse_lex.c -I../inc -I../../../dsa/include

ds_parse_yacc.o : ds_parse_yacc.c
	gcc -g -c ds_parse_yacc.c -I../inc -I../../../dsa/include

ds_symbol_mgr.o : ds_symbol_mgr.c
	gcc -g -c ds_symbol_mgr.c -I../inc -I../../../dsa/include

#clean :
#	rm *.o
