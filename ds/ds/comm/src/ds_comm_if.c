/*****************************************************************************/
/** 
* \file       ds_comm_if.c
* \author     hejian
* \date       2015/06/04
* \version    FlexBNG V1
* \brief      �ṩ���ڲ�DSS��DSAʹ�õĽӿ�
* \note       Copyright (c) 2000-2020  ����˹��Ϣ�Ƽ��ɷ����޹�˾
* \remarks    �޸���־
******************************************************************************/
#include "ds_comm_incl.h"

INT32  ds_comm_parse_init(VOID)
{
    INT32  ret = TRUE;

    /*Init Symbol manager*/
    ds_symbol_mgr_init();
    
    return  ret;
}

INT32  ds_comm_parse_run(CHAR *line)
{
    INT32  ret = -1;

	ds_yystart(line);
	ret = ds_yyparse();
	
	return  ret;
}
