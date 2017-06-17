/*****************************************************************************/
/** 
* \file       dss_am_msg.c
* \author     hejian
* \date       2015/06/06
* \version    FlexBNG V1
* \brief      Agent Manager Message Process Header
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#ifndef  _DS_AM_MSG_H_
#define  _DS_AM_MSG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define  DSS_MSG_BUF_LEN  2048

INT32  dss_am_msg_rcv(DSS_AM_AGENT *agent, INT32  fd);

#ifdef __cplusplus
}
#endif

#endif
