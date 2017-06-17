/*****************************************************************************/
/** 
* \file       ds_comm_if.c
* \author     hejian
* \date       2015/06/04
* \version    FlexBNG V1
* \brief      提供给内部DSS和DSA使用的接口
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
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
