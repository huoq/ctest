/*****************************************************************************/
/** 
* \file       ds_comm_if.h
* \author     hejian
* \date       2015/06/04
* \version    FlexBNG V1
* \brief      �ṩ���ڲ�DSS��DSAʹ�õĽӿ�ͷ�ļ�
* \note       Copyright (c) 2000-2020  ����˹��Ϣ�Ƽ��ɷ����޹�˾
* \remarks    �޸���־
******************************************************************************/
#ifndef _DS_COMM_IF_H_
#define _DS_COMM_IF_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*ds comm����(DSS��DSA)���ֵĹ��ܽӿ�*/

INT32  ds_comm_parse_init(VOID);
INT32  ds_comm_parse_run(CHAR *line);
INT32  ds_symbol_add(CHAR *name, CHAR *value, INT32 type);

#ifdef __cplusplus
}
#endif
#endif
