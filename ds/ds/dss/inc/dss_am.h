/*****************************************************************************/
/** 
* \file       dss_am.h
* \author     hejian
* \date       2015/06/05
* \version    FlexBNG V1
* \brief      Agent Manager Header
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#ifndef  _DSS_AM_H_
#define  _DSS_AM_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    DSS_AGENT_REG_RCV = 1,
    DSS_AGENT_REG_ACK_SND,
    DSS_AGENT_OUTPUT_RCV
}DSS_AGENT_EVENT;

typedef  enum
{
    DSS_STATE_INIT = 1,    /*Init State*/
    DSS_STATE_REG_RCV, /*Reg msg have rcv from agent*/
    DSS_STATE_ACTIVE,  /*After Reg ack msg has send to agent*/
}DSS_AGENT_STATE;

typedef  struct tagDSS_AM_AGENT
{
    UINT32  os_pid;
    CHAR    proc_name[DS_PROC_NAME_LEN];
    INT32   agent_fd;
    LIST_ENTRY(tagDSS_AM_AGENT) agent_entry;
    DSS_AGENT_STATE   cur_state;
}DSS_AM_AGENT;


INT32  dss_am_init(VOID);
DSS_AM_AGENT*  dss_am_agent_find_by_pid(UINT32 ospid);
INT32  dss_am_agent_exist_by_pid(UINT32 ospid);
INT32  dss_am_agent_exist_by_name(const CHAR  *proc_name);
INT32  dss_am_agent_name2pid(const CHAR  *proc_name, UINT32 *ppid);
INT32  dss_am_agent_pid2name(UINT32 pid, CHAR *proc_name);

VOID  dss_am_agent_del(INT32  fd); /*dirty design*/
INT32  dss_am_agent_fsm_input(DSS_AM_AGENT *agent, DSS_AGENT_EVENT fsm_event);

VOID  dss_am_allproc_show(VOID);
#ifdef __cplusplus
}
#endif

#endif

