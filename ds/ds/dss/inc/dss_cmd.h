/*****************************************************************************/
/** 
* \file       dss_am_msg.c
* \author     hejian
* \date       2015/06/06
* \version    FlexBNG V1
* \brief      Command Process Header
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#ifndef  _DSS_CMD_H_
#define  _DSS_CMD_H_

#ifdef  __cplusplus
extern "C"
{
#endif

INT32  dss_cmd_init(VOID);
INT32  dss_cmd_dispatch(DSS_TSESSION *ts, CHAR *cmd_str);

INT32  dss_am_msg_cmd_snd(DSS_TSESSION  *ts, const CHAR *msg_str);

#ifdef  __cplusplus
#endif
#endif
