/*****************************************************************************/
/** 
* \file       ds_comm_util.h
* \author     hejian
* \date       2015/06/06
* \version    FlexBNG V1
* \brief      �������õĺ���ʵ��ͷ�ļ�
* \note       Copyright (c) 2000-2020  ����˹��Ϣ�Ƽ��ɷ����޹�˾
* \remarks    �޸���־
******************************************************************************/
#ifndef  _DS_COMM_UTIL_H_
#define  _DS_COMM_UTIL_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#define  DS_FD_SET_NONBLK(FD) \
    fcntl(FD, F_SETFL, fcntl(FD,F_GETFL) | O_NONBLOCK);

INT32  ds_util_arg_get(CHAR *in_str, INT32 index, CHAR *arg_str, INT32 arg_len);
CHAR*  ds_comm_trim(CHAR *in_buf);


#ifdef  __cplusplus
}
#endif
#endif
