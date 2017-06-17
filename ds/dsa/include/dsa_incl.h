/*****************************************************************************/
/** 
* \file       dsa_incl.h
* \author     hejian
* \date       2015/06/06
* \version    FlexBNG V1
* \brief      DSAģ�����ͷ�ļ�
* \note       Copyright (c) 2000-2020  ����˹��Ϣ�Ƽ��ɷ����޹�˾
* \remarks    �޸���־
******************************************************************************/
#ifndef  _DSA_INCL_H_
#define  _DSA_INCL_H_

#ifdef __cplusplus
extern "C"
{
#endif


/* �ⲿ����ͷ�ļ� */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <arpa/telnet.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>
#include <pty.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include "queue.h"

/* �ڲ�����ͷ�ļ� */
#include "dsa_export.h"
#include "ds_comm_type.h"
#include "ds_comm_def.h"
#include "ds_comm_if.h"
#include "ds_comm_util.h"
#include "ds_comm_cmd.h"
#include "ds_comm_event.h"
#include "ds_comm_msg.h"
#include "dsa_io.h"
#include "dsa_cmd.h"
#include "dsa_msg.h"
#include "dsa_parse_elf.h"


/* һЩ���ö��� */
#define  DSA_CODE(x)  1

#ifdef __cplusplus
}
#endif

#endif
