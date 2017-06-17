/*****************************************************************************/
/** 
* \file       dss_am.c
* \author     hejian
* \date       2015/06/05
* \version    FlexBNG V1
* \brief      Agent Manager
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "dss_incl.h"

/*There are 100 agents at the same time*/
#define  DSS_AM_MAX_EVENT  100

typedef  struct tagDSS_AM_GLOBAL
{
    INT32  master_fd;
    DS_EVENT_MNG *event_mng;
    LIST_HEAD(tagDSS_AGENT_LIST, tagDSS_AM_AGENT) agent_list;
}DSS_AM_GLOABL;

DSS_AM_GLOABL  g_dss_am_global;

#if  DSS_CODE("Agent状态机")
INT32  dss_am_agent_fsm_input(DSS_AM_AGENT *agent, DSS_AGENT_EVENT fsm_event)
{
    ds_assert(agent);

    switch (fsm_event)
    {
        case  DSS_AGENT_REG_RCV:
            if (agent->cur_state != DSS_STATE_INIT)
            {
                ds_assert(0);
                goto  FAIL_LABEL;
            }
            agent->cur_state = DSS_STATE_REG_RCV;
            break;
        case  DSS_AGENT_REG_ACK_SND:
            if (agent->cur_state != DSS_STATE_REG_RCV)
            {
                ds_assert(0);
                goto  FAIL_LABEL;
            }
            agent->cur_state = DSS_STATE_ACTIVE;
            break;
        case  DSS_AGENT_OUTPUT_RCV:
            if (agent->cur_state != DSS_STATE_ACTIVE)
            {
                ds_assert(0);
                goto  FAIL_LABEL;
            }
            break;
        default:
            ds_assert(0);
            goto  FAIL_LABEL;
            break;
    }
    
    return  TRUE;
FAIL_LABEL:
    return  FALSE;
}
#endif

#if  DSS_CODE("Agent管理")
DSS_AM_AGENT*  dss_am_agent_find_by_fd(INT32  fd)
{
    DSS_AM_AGENT  *am_agent = NULL;
    
    LIST_FOREACH(am_agent, &(g_dss_am_global.agent_list), agent_entry)
    {
        if (am_agent->agent_fd == fd)
        {
            ds_assert(am_agent->cur_state >= DSS_STATE_INIT);
            break;
        }
    }
    
    return  am_agent;
}

DSS_AM_AGENT*  dss_am_agent_new(INT32  fd)
{
    DSS_AM_AGENT  *new_agent = NULL;
   
    new_agent = malloc(sizeof(DSS_AM_AGENT));
    if (new_agent == NULL)
    {
        ds_assert(0);
        goto  FAIL_LABEL;
    }
    memset(new_agent, 0, sizeof(DSS_AM_AGENT));

    new_agent->agent_fd = fd;
    new_agent->cur_state = DSS_STATE_INIT;
    LIST_INSERT_HEAD(&(g_dss_am_global.agent_list), new_agent, agent_entry);
    
FAIL_LABEL:
    return  new_agent;
}

VOID  dss_am_agent_del(INT32  fd)
{
    DSS_AM_AGENT  *am_agent = NULL;

    am_agent = dss_am_agent_find_by_fd(fd);
    if (am_agent == NULL)
    {
        ds_assert(0);
        goto  FAIL_LABEL;
    }

    ds_log("Delete agent: %p,%d", am_agent, fd);
    ds_event_mng_del_event(g_dss_am_global.event_mng, fd, DS_EVENT_IN);
    close(fd);
    LIST_REMOVE(am_agent, agent_entry);
    free(am_agent);

FAIL_LABEL:
    return;
}

DSS_AM_AGENT*  dss_am_agent_find_by_name(const CHAR  *proc_name)
{
    DSS_AM_AGENT  *am_agent = NULL;
    
    LIST_FOREACH(am_agent, &(g_dss_am_global.agent_list), agent_entry)
    {
        if (!strcasecmp(am_agent->proc_name, proc_name))
        {
            ds_assert(am_agent->cur_state >= DSS_STATE_INIT);
            break;
        }
    }
    
    return  am_agent;
}


VOID  dss_am_agent_fd_handler(INT32  fd)
{
    DSS_AM_AGENT  *am_agent = NULL;

    am_agent = dss_am_agent_find_by_fd(fd);
    if (am_agent == NULL)
    {
        ds_assert(0);
        goto  FAIL_LABEL;
    }
    
    /*Receive msg from agent*/
    dss_am_msg_rcv(am_agent, fd);

FAIL_LABEL:
    return;

}

#endif

#if  DSS_CODE("内部接口")

VOID  dss_am_server_listen_fd_handler(INT32  fd)
{
    INT32 agent_fd = -1;
    
    ds_assert(fd == g_dss_am_global.master_fd);
    while ((agent_fd = accept(fd, 0, 0)) > 0)
    {
        DS_FD_SET_NONBLK(agent_fd);

        ds_log("Agent %d comming\n", fd);

        ds_event_mng_add_event(g_dss_am_global.event_mng, agent_fd,
            DS_EVENT_IN, dss_am_agent_fd_handler);

        dss_am_agent_new(agent_fd);
    }
    if (agent_fd == -1 )
    {
        if (errno != EAGAIN && errno != ECONNABORTED 
            && errno != EPROTO && errno != EINTR)
         {
             ds_log_err("Accept failed\n");
             return;
         }
    }
    
    return;
}

INT32  dss_am_server_listen(VOID)
{
    struct  sockaddr_un  un;
    int len = 0;


_TRAY_AGAIN_:
    sleep(5);
    g_dss_am_global.master_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (g_dss_am_global.master_fd < 0)
    {
        ds_log_err("Create am listen socket failed\n");
        goto _TRAY_AGAIN_;
    }

    unlink(DS_AM_LISTEN_NAME);
    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, DS_AM_LISTEN_NAME);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(DS_AM_LISTEN_NAME);

    DS_FD_SET_NONBLK(g_dss_am_global.master_fd);

    if (bind(g_dss_am_global.master_fd, (struct sockaddr *)&un, len) < 0)
    {
        ds_log_err("Bind am listen socket failed\n");
        close(g_dss_am_global.master_fd);
        goto _TRAY_AGAIN_;
    }

    if (listen(g_dss_am_global.master_fd, 1))
    {
        ds_log_err("Listen on am listen socket failed\n");
        close(g_dss_am_global.master_fd);
        goto _TRAY_AGAIN_;
    }

    ds_event_mng_add_event(g_dss_am_global.event_mng, g_dss_am_global.master_fd,
                           DS_EVENT_IN, dss_am_server_listen_fd_handler);
    /*There come endless loop~~*/                       
    ds_event_mng_process(g_dss_am_global.event_mng);
    
    return  TRUE;
}

VOID* dss_am_server_thread_handler(VOID *param)
{
    dss_am_server_listen();

    return  NULL;
}

INT32  dss_am_server_thread_start(VOID)
{
    INT32  ret = -1;
    pthread_t am_tid;
    
    ret = pthread_create(&am_tid, NULL, dss_am_server_thread_handler, NULL);
    if (ret == -1)
    {
        goto FAIL_LABEL;
    }

    /* detach for resource free */
    pthread_detach(am_tid);

FAIL_LABEL:
    return  ret;
}
#endif
DSS_AM_AGENT*  dss_am_agent_find_by_pid(UINT32 ospid)
{
    DSS_AM_AGENT  *am_agent = NULL;
    
    LIST_FOREACH(am_agent, &(g_dss_am_global.agent_list), agent_entry)
    {
        if (am_agent->os_pid == ospid)
        {
            ds_assert(am_agent->cur_state >= DSS_STATE_INIT);
            break;
        }
    }
    
    return  am_agent;
}


INT32  dss_am_agent_exist_by_name(const CHAR  *proc_name)
{
    DSS_AM_AGENT  *am_agent = NULL;
    
    am_agent = dss_am_agent_find_by_name(proc_name);
    return  (am_agent != NULL);
}

INT32  dss_am_agent_exist_by_pid(UINT32 ospid)
{
    DSS_AM_AGENT  *am_agent = NULL;
    
    am_agent = dss_am_agent_find_by_pid(ospid);
    return  (am_agent != NULL);
}


INT32  dss_am_agent_name2pid(const CHAR  *proc_name, UINT32 *ppid)
{
    DSS_AM_AGENT  *am_agent = NULL;

    ds_assert(ppid);
    am_agent = dss_am_agent_find_by_name(proc_name);
    if (am_agent == NULL)
    {
        return  FALSE;
    }

    *ppid = am_agent->os_pid;
    
    return  TRUE;
}


INT32  dss_am_agent_pid2name(UINT32 pid, CHAR *proc_name)
{
    DSS_AM_AGENT  *am_agent = NULL;

    ds_assert(proc_name);
    am_agent = dss_am_agent_find_by_pid(pid);
    if (am_agent == NULL)
    {
        return  FALSE;
    }

    strncpy(proc_name, am_agent->proc_name, DS_PROC_NAME_LEN);
    
    return  TRUE;
}


INT32  dss_am_init(VOID)
{
    /*Init global struct*/
    memset(&g_dss_am_global, 0, sizeof(DSS_AM_GLOABL));
    g_dss_am_global.event_mng = ds_event_mng_create(DSS_AM_MAX_EVENT);
    if (g_dss_am_global.event_mng == NULL)
    {
        return  FALSE;
    }
    
    LIST_INIT(&(g_dss_am_global.agent_list));

    /*Start server thread*/
    dss_am_server_thread_start();
    
    return  TRUE;
}

VOID  dss_am_allproc_show(VOID)
{
    DSS_AM_AGENT  *am_agent = NULL;
    
    DS_PRINTF("\npid     proc_name    \n");
    DS_PRINTF("----------------------\n");

    LIST_FOREACH(am_agent, &(g_dss_am_global.agent_list), agent_entry)
    {
        DS_PRINTF("%-8u%-s\n",
                  am_agent->os_pid,
                  am_agent->proc_name);
    }
    
    return;
}
