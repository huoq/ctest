/*****************************************************************************/
/** 
* \file       ds_parse_lex.c
* \author     hejian
* \date       2015/06/04
* \version    FlexBNG V1
* \brief      �ʷ�������ͷ�ļ�
* \note       Copyright (c) 2000-2020  ����˹��Ϣ�Ƽ��ɷ����޹�˾
* \remarks    �޸���־
******************************************************************************/
#ifndef _DS_PARSE_LEX_H_
#define _DS_PARSE_LEX_H_

#ifdef __cplusplus
extern "C"
{
#endif

int64_t ds_yylex(VOID);
VOID ds_lexNewLine(CHAR *line);

#ifdef __cplusplus
}
#endif
#endif
