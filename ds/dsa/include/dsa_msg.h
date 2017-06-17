/*****************************************************************************/
/** 
* \file       dsa_msg.h
* \author     hejian
* \date       2015/06/08
* \version    FlexBNG V1
* \brief      处理从DSS过来的消息头文件
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/

#ifndef  _DSA_MSG_H_
#define  _DSA_MSG_H_

#ifdef  __cplusplus
extern "C"
{
#endif

typedef enum
{
    DSA_CONN_CONNECTED = 1,
    DSA_CONN_REG_ACK_RCV,
    DSA_CONN_CMD_MSG_RCV,
    DSA_CONN_DESTROY
}DSA_CONN_EVENT;


INT32  dsa_msg_init(VOID);
VOID   dsa_msg_uninit(VOID);
INT32  dsa_msg_rcv(INT32 fd);
VOID   dsa_msg_set_peer_fd(INT32  fd);
INT32  dsa_conn_fsm_input(DSA_CONN_EVENT fsm_event);
INT32  dsa_msg_snd_output(UINT8 *out_buf, INT32 buf_len);

#ifdef  __cplusplus
}
#endif
#endif

