/*****************************************************************************/
/** 
* \file       ds_parse_yacc.h
* \author     hejian
* \date       2015/06/05
* \version    FlexBNG V1
* \brief      语法分析器头文件
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#ifndef _DS_PARSE_YACC_H_
#define _DS_PARSE_YACC_H_

#ifdef __cplusplus
extern "C"
{
#endif

VOID ds_yystart(CHAR *line);
INT32 ds_yyparse(VOID);

#ifdef __cplusplus
}
#endif
#endif

