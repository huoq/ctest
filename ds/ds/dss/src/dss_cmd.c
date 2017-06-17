/*****************************************************************************/
/** 
* \file       dss_am_msg.c
* \author     hejian
* \date       2015/06/06
* \version    FlexBNG V1
* \brief      Command Process
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "dss_incl.h"

#if  DSS_CODE("命令结构")
DS_CMD_LIST  g_dss_admin_cmd_list;
DS_CMD_LIST  g_dss_comm_cmd_list;

DS_CMD_LIST* dss_cmd_list_get(UINT32 mode)
{
    DS_CMD_LIST  *tmp_list = NULL;

    switch (mode)
    {
        case  SESS_MODE_ADMIN:
            tmp_list = &g_dss_admin_cmd_list;
            break;
        case  SESS_MODE_DUMMY:
            tmp_list = &g_dss_comm_cmd_list;
            break;
        default:
            ds_assert(0);
            break;
    }

    return  tmp_list;
}
#endif

#if DSS_CODE("命令执行函数")
VOID dss_cmd_handler_show_proc(CHAR *cmd_str, DSS_TSESSION *ts)
{
    dss_am_allproc_show();
    return;
}

VOID dss_cmd_handler_switchto_proc(CHAR *cmd_str, DSS_TSESSION *ts)
{
    CHAR  pid_str[32] = {0};
    UINT32 os_pid = 0;
    DSS_SESS_CTX *psess_ctx = NULL;

    if (!ds_util_arg_get(cmd_str, 2, pid_str, sizeof(pid_str)))
    {
        goto FAIL_LABEL;
    }

    if (!dss_am_agent_name2pid(pid_str, &os_pid))
    {
        os_pid = strtod(pid_str, NULL);
    }
    
    ds_log("pid_str:%s, pid:%u\n", pid_str, os_pid);
    
    if (os_pid == 0)
    {
        goto FAIL_LABEL;
    }

    if (!dss_am_agent_exist_by_pid(os_pid))
    {
        goto FAIL_LABEL;
    }

    /*Enter process mode*/
    psess_ctx = dss_sess_get_ctx(ts);
    if (psess_ctx == NULL)
    {
        goto FAIL_LABEL;
    }
    dss_sess_ctx_set_proc(psess_ctx, os_pid);
    return;
    
FAIL_LABEL:
    DS_PRINTF("Failed to switch to proc %s\n", pid_str);
    return;
}

/* Exit current mode */
VOID dss_cmd_handler_exit(CHAR *cmd_str, DSS_TSESSION *ts)
{
    DSS_SESS_CTX *sess_ctx = NULL;
    SESS_MODE cur_mode;
    
    ds_assert(cmd_str && ts);

    sess_ctx = dss_sess_get_ctx(ts);
    if (sess_ctx == NULL)
    {
        ds_assert(0);
        return;
    }

    dss_sess_ctx_get_mode(sess_ctx, &cur_mode);
    
    switch (cur_mode)
    {
        case SESS_MODE_ADMIN:
            break;
        case SESS_MODE_PROC:
            dss_sess_ctx_set_admin(sess_ctx);
            break;
        default:
            ds_assert(0);
            break;
    }
    
    return ;
}

VOID  dss_cmd_handler_help(CHAR *cmd_str, DSS_TSESSION *ts)
{
    ds_cmd_list_print(&g_dss_admin_cmd_list);

    return;
}

/* Command under admin mode */
DS_CMD_ENTRY  g_dss_admin_command[] = 
{
    {"show proc", "show proc",
        "display all process in current system",
        (DS_CMD_HANDLER)dss_cmd_handler_show_proc},
    {"proc", "p",
        "switch to specific process",
        (DS_CMD_HANDLER)dss_cmd_handler_switchto_proc},
    {"help", "h",
        "print help info",
        (DS_CMD_HANDLER)dss_cmd_handler_help}
};

/* Command under common mode */
DS_CMD_ENTRY  g_dss_comm_command[] = 
{
    {"exit", "exit",
        "exit current mode",
        (DS_CMD_HANDLER)dss_cmd_handler_exit},
    {"quit", "q",
        "logout",
        NULL}
};
#endif

UINT32 dss_cmd_admin_mode_exec(DSS_TSESSION *ts, CHAR *cmd_str)
{
    DS_CMD_HANDLER  func_handler = NULL;

    
    func_handler = ds_cmd_handler_find(&g_dss_admin_cmd_list, cmd_str);
    if (func_handler != NULL)
    {
        func_handler(cmd_str, ts);
        return  TRUE;
    }
 
    return  FALSE;
}

INT32 dss_cmd_proc_mode_exec(DSS_TSESSION *ts, CHAR *cmd_str)
{
    /*Under process mode, any cmd will be send to agent*/
    dss_am_msg_cmd_snd(ts, cmd_str);
 
    return  TRUE;
}

INT32 dss_cmd_comm_mode_exec(DSS_TSESSION *ts, CHAR *cmd_str)
{
    DS_CMD_HANDLER  func_handler = NULL;

    func_handler = ds_cmd_handler_find(&g_dss_comm_cmd_list, cmd_str);
    if (func_handler != NULL)
    {
        func_handler(cmd_str, ts);
        return  TRUE;
    }
 
    return  FALSE;
}

UINT32 dss_cmd_prev_exec(DSS_TSESSION *ts, CHAR *cmd_str)
{
    CHAR  first_cmd_str[DS_MAX_CHARS_PER_LINE] = {0};

    ds_assert(ts);
    ds_assert(cmd_str);

    if (!ds_util_arg_get(cmd_str, 1, first_cmd_str, sizeof(first_cmd_str)))
    {
        goto EXIT_LABEL;
    }

    if (first_cmd_str[0] == '\0')
    {
        goto EXIT_LABEL;
    }

    if (DS_STR_MATCH(first_cmd_str, "quit") || DS_STR_MATCH(first_cmd_str, "q"))
    {
        return TRUE;
    }
    
EXIT_LABEL:
    return  FALSE;
}

INT32  dss_cmd_dispatch(DSS_TSESSION *ts, CHAR *cmd_str)
{
    DSS_SESS_CTX *sess_ctx = NULL;
    SESS_MODE cur_mode;
    
    if (dss_cmd_prev_exec(ts, cmd_str))
    {
        goto EXIT_LABEL;
    }

    /**/
    if (dss_cmd_comm_mode_exec(ts, cmd_str))
    {
        return TRUE;
    }
    
    sess_ctx = dss_sess_get_ctx(ts);
    if (sess_ctx == NULL)
    {
        ds_assert(0);
        goto EXIT_LABEL;
    }

    dss_sess_ctx_get_mode(sess_ctx, &cur_mode);
    switch (cur_mode)
    {
        case  SESS_MODE_ADMIN:
            if (dss_cmd_admin_mode_exec(ts, cmd_str))
            {
                return TRUE;
            }
            break;
        case  SESS_MODE_PROC:
            if (dss_cmd_proc_mode_exec(ts, cmd_str))
            {
                return TRUE;
            }
            break;
        default:
            break;
    }    

    /* Finally,run parser! */
    ds_comm_parse_run(cmd_str);
    
    return TRUE;

EXIT_LABEL:
    return FALSE;
}

INT32  dss_cmd_init(VOID)
{
    TAILQ_INIT(&g_dss_admin_cmd_list.cmd_head);
    TAILQ_INIT(&g_dss_comm_cmd_list.cmd_head);

    /*Install command for Admin mode*/
    if (!ds_cmd_install(&g_dss_admin_cmd_list, g_dss_admin_command,
        sizeof(g_dss_admin_command)/sizeof(DS_CMD_ENTRY))
        || !ds_cmd_install(&g_dss_admin_cmd_list, g_dss_comm_command,
        sizeof(g_dss_comm_command)/sizeof(DS_CMD_ENTRY))
       )
    {
        return  FALSE;
    }

    /*Install command for comm mode*/
    if (!ds_cmd_install(&g_dss_comm_cmd_list, g_dss_comm_command,
        sizeof(g_dss_comm_command)/sizeof(DS_CMD_ENTRY)))
    {
        return  FALSE;
    }

    return  TRUE;
}
