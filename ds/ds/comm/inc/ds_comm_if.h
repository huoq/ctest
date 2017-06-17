/*****************************************************************************/
/** 
* \file       ds_comm_if.h
* \author     hejian
* \date       2015/06/04
* \version    FlexBNG V1
* \brief      提供给内部DSS和DSA使用的接口头文件
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#ifndef _DS_COMM_IF_H_
#define _DS_COMM_IF_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*ds comm对外(DSS和DSA)呈现的功能接口*/

INT32  ds_comm_parse_init(VOID);
INT32  ds_comm_parse_run(CHAR *line);
INT32  ds_symbol_add(CHAR *name, CHAR *value, INT32 type);

#ifdef __cplusplus
}
#endif
#endif
