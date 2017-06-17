/*****************************************************************************/
/** 
* \file       dss_sess_histroy.h
* \author     hejian
* \date       2015/06/02
* \version    FlexBNG V1
* \brief      telnet服务器头文件
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#ifndef  _DSS_TELNETD_H_
#define  _DSS_TELNETD_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define TS_BUF1 ((UINT8*)(ts + 1))
#define TS_BUF2 (((UINT8*)(ts + 1)) + BUFSIZE)

#define ALIGN1 __attribute__((aligned(1)))

#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)

struct tsession;
typedef struct tsession DSS_TSESSION;


/*对外提供的接口*/
INT32  dss_telnetd_start(VOID);
DSS_SESS_CTX*  dss_sess_get_ctx(struct tsession  *ts);
INT32  dss_sess_get_idx(struct tsession  *ts);
VOID  dss_sess_write_term(INT32 sess_index, UINT8 *msg, UINT32 msg_len);
VOID  dss_sess_write_all_term(UINT8 *msg, UINT32 msg_len);

#ifdef __cplusplus
}
#endif

#endif
