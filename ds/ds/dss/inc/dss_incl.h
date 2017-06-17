#ifndef  _DSS_INCL_H_
#define  _DSS_INCL_H_

#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************/
/** 
* \file       dss_incl.h
* \author     hejian
* \date       2015/06/02
* \version    FlexBNG V1
* \brief      DSS include header files
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
/* 外部包含头文件 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
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
#include "queue.h"
#include "tree.h"

/* 内部包含头文件 */
#include "cos_ds_if.h"
#include "ds_comm_type.h"
#include "ds_comm_def.h"
#include "ds_comm_if.h"
#include "ds_comm_util.h"
#include "ds_comm_cmd.h"
#include "ds_comm_event.h"
#include "ds_comm_msg.h"
#include "dss_term_key.h"
#include "dss_sess_histroy.h"
#include "dss_sess_ctx.h"
#include "dss_telnetd.h"
#include "dss_am.h"
#include "dss_am_msg.h"
#include "dss_cmd.h"


/* 一些公用定义 */
#define  DSS_CODE(x)  1

#ifdef __cplusplus
}
#endif

#endif
