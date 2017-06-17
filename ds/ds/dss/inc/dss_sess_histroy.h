/*****************************************************************************/
/** 
* \file       dss_sess_histroy.h
* \author     hejian
* \date       2015/06/10
* \version    FlexBNG V1
* \brief      ���¼��л���ʷ�������ͷ�ļ�
* \note       Copyright (c) 2000-2020  ����˹��Ϣ�Ƽ��ɷ����޹�˾
* \remarks    �޸���־
******************************************************************************/
#ifndef  _DSS_SESS_HISTROY_H_
#define  _DSS_SESS_HISTROY_H_

#ifdef  __cplusplus
extern "C"
{
#endif

struct tagDSS_SESS_HISTROY;
typedef struct tagDSS_SESS_HISTROY DSS_SESS_HISTROY;

DSS_SESS_HISTROY*  dss_histroy_init(UINT32 max_cmd_num);
INT32  dss_histroy_insert(DSS_SESS_HISTROY *sess_his, CHAR *cmd_str);
INT32  dss_histroy_prev(DSS_SESS_HISTROY *sess_his, CHAR **ppcmd_str);
INT32  dss_histroy_next(DSS_SESS_HISTROY *sess_his, CHAR **ppcmd_str);
VOID    dss_histroy_uninit(DSS_SESS_HISTROY *sess_his);

#ifdef  __cplusplus
}
#endif

#endif
