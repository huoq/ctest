/*****************************************************************************/
/** 
* \file       dsa_cmd.c
* \author     hejian
* \date       2015/06/08
* \version    FlexBNG V1
* \brief      处理从用户端过来的命令
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "dsa_incl.h"

#if  DSA_CODE("内部接口")
DS_CMD_LIST  g_dsa_proc_cmd_list;
pthread_mutex_t g_cmd_wait_mux;


VOID  dsa_cmd_handler_help(CHAR *cmd_str, VOID *ts)
{
    ds_cmd_list_print(&g_dsa_proc_cmd_list);

    return;
}

/* Command under admin mode */
DS_CMD_ENTRY  g_dsa_proc_command[] = 
{
    {"help", "h",
        "print help info",
        (DS_CMD_HANDLER)dsa_cmd_handler_help}
};

INT32 dsa_cmd_proc_exec(CHAR *cmd_str)
{
    DS_CMD_HANDLER  func_handler = NULL;

    func_handler = ds_cmd_handler_find(&g_dsa_proc_cmd_list, cmd_str);
    if (func_handler != NULL)
    {
        func_handler(cmd_str, NULL);
        return  TRUE;
    }
 
    return  FALSE;
}

VOID* dsa_cmd_execute_thread(VOID *param)
{
    CHAR *cmd_str = param;
    INT32  locked = -1;

    locked = pthread_mutex_trylock(&g_cmd_wait_mux);
    if (locked == EBUSY)
    {
        sleep(5);
    }
    
    /*Try to find if the cmd_str match the command under proc mode*/
    if (dsa_cmd_proc_exec(cmd_str))
    {
        goto EXIT_LABEL;
    }

    /* Finally,run parser! */
    ds_comm_parse_run(cmd_str);
    
EXIT_LABEL:
    if (locked == 0)
    {
        pthread_mutex_unlock(&g_cmd_wait_mux);
    }
    return NULL;
}
#endif

INT32  dsa_cmd_dispatch(CHAR *cmd_str)
{
    INT32  ret = -1;
    pthread_t am_tid;
    
    ret = pthread_create(&am_tid, NULL, dsa_cmd_execute_thread, cmd_str);
    if (ret == -1)
    {
        goto EXIT_LABEL;
    }

    /* detach for resource free */
    pthread_detach(am_tid);
    
EXIT_LABEL:
    return TRUE;
}


INT32  dsa_cmd_init(VOID)
{
    TAILQ_INIT(&g_dsa_proc_cmd_list.cmd_head);

    /*Install command for proc mode*/
    if (!ds_cmd_install(&g_dsa_proc_cmd_list, g_dsa_proc_command,
        sizeof(g_dsa_proc_command)/sizeof(DS_CMD_ENTRY)))
    {
        return  FALSE;
    }

    pthread_mutex_init(&g_cmd_wait_mux, NULL);
    
    return  TRUE;
}

