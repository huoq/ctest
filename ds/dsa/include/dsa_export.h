/*****************************************************************************/
/** 
* \file       ds_if.h
* \author     hejian
* \date       2015/06/02
* \version    FlexBNG V1
* \brief      �����ṩ�Ľӿ�
* \note       Copyright (c) 2000-2020  ����˹��Ϣ�Ƽ��ɷ����޹�˾
* \remarks    �޸���־
******************************************************************************/
#ifndef _COS_DS_IF_H_
#define _COS_DS_IF_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*Debugging Shell Agent Initialization*/
void *  dsa_init(void);

/*Debugging Shell output func*/
int  DS_PRINTF(const char *fmt, ...);

#ifdef  __cplusplus
}
#endif
#endif
