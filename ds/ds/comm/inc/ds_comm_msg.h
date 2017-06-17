/*****************************************************************************/
/** 
* \file       ds_comm_msg.h
* \author     hejian
* \date       2015/06/06
* \version    FlexBNG V1
* \brief      DSS和DSA之间的通信消息结构定义
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/

#ifndef  _DS_COMM_MSG_H_
#define  _DS_COMM_MSG_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*Maximum msg length per msg*/
#define  DS_MSG_BUF_LEN  4096

typedef enum
{
    DS_MSG_TYPE_REG,    /*reg msg, include ack*/
    DS_MSG_TYPE_CMD,    /*dss send to dsa*/
    DS_MSG_TYPE_OUTPUT  /*dsa send to dss*/
}DS_MSG_TYPE;

typedef  struct tagDS_MSG_HEADER
{
    DS_MSG_TYPE  type;
    UINT32  len;  /*include header length*/
}DS_MSG_HEADER;

typedef  struct tagDS_MSG_REG
{
    DS_MSG_HEADER head;
    UINT32  os_pid;
    CHAR  proc_name[DS_PROC_NAME_LEN];
}DS_MSG_REG;

typedef  struct tagDS_MSG_CMD
{
    DS_MSG_HEADER head;
    INT32  sess_index;
    CHAR   cmd_str[];
}DS_MSG_CMD;

typedef  struct tagDS_MSG_OUTPUT
{
    DS_MSG_HEADER head;
    INT32  sess_index;
    UINT8   output[];
}DS_MSG_OUTPUT;

#define  DS_MSG_OUTPUT_BUF_LEN  (DS_MSG_BUF_LEN - sizeof(DS_MSG_OUTPUT))

#ifdef __cplusplus
}
#endif
#endif
