/*****************************************************************************/
/** 
* \file       ds_parse_yacc.h
* \author     hejian
* \date       2015/06/05
* \version    FlexBNG V1
* \brief      �﷨������ͷ�ļ�
* \note       Copyright (c) 2000-2020  ����˹��Ϣ�Ƽ��ɷ����޹�˾
* \remarks    �޸���־
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

