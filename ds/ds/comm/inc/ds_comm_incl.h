#ifndef _DS_COMM_INCL_H_
#define _DS_COMM_INCL_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* 外部包含头文件 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <dlfcn.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <errno.h>
#include <inttypes.h>
/*****************************************************************************/
/** 
* \file       ds_comm_incl.h
* \author     hejian
* \date       2015/06/05
* \version    FlexBNG V1
* \brief      DS COMM内部的包含头文件
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "queue.h"
#include "tree.h"

/* 内部包含头文件 */
#include "dsa_export.h"
#include "ds_comm_type.h"
#include "ds_comm_def.h"
#include "ds_comm_cmd.h"
#include "ds_comm_util.h"
#include "ds_comm_event.h"
#include "ds_parse.h"
#include "ds_symbol_mgr.h"
#include "ds_parse_lex.h"
#include "ds_parse_yacc.h"


/* 一些公用定义 */
#define  DS_CODE(x)  1

#ifdef __cplusplus
}
#endif
#endif
