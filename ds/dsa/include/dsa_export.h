/*****************************************************************************/
/** 
* \file       ds_if.h
* \author     hejian
* \date       2015/06/02
* \version    FlexBNG V1
* \brief      对外提供的接口
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
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
