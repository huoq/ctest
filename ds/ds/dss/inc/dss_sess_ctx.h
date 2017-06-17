/*****************************************************************************/
/** 
* \file       dss_sess_ctx.h
* \author     hejian
* \date       2015/06/03
* \version    FlexBNG V1
* \brief      �û�ִ������ģʽͷ�ļ�
* \note       Copyright (c) 2000-2020  ����˹��Ϣ�Ƽ��ɷ����޹�˾
* \remarks    �޸���־
******************************************************************************/
#ifndef  _DSS_SESS_CTX_H_
#define  _DSS_SESS_CTX_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*�Ự���ڵ�ģʽ*/
typedef  enum
{
    SESS_MODE_ADMIN, /* ����ģʽ */
    SESS_MODE_PROC,  /*����ģʽ*/
    SESS_MODE_MAX,
    SESS_MODE_DUMMY
}SESS_MODE;


struct tagDSS_SESS_CTX;
typedef struct tagDSS_SESS_CTX  DSS_SESS_CTX;


INT32  dss_sess_ctx_init(DSS_SESS_CTX **ppsess_ctx);
VOID   dss_sess_ctx_uninit(DSS_SESS_CTX **ppsess_ctx);
INT32  dss_sess_ctx_set_admin(DSS_SESS_CTX *psess_ctx);
INT32  dss_sess_ctx_set_proc(DSS_SESS_CTX *psess_ctx, UINT32 ospid);
VOID   dss_sess_ctx_get_mode(DSS_SESS_CTX *psess_ctx, SESS_MODE *mode);
VOID   dss_sess_ctx_get_mode_str(DSS_SESS_CTX *psess_ctx, CHAR *mode_str);
VOID   dss_sess_ctx_get_ospid(DSS_SESS_CTX *psess_ctx, UINT32 *ospid);


#ifdef __cplusplus
}
#endif

#endif
