/*****************************************************************************/
/** 
* \file       dss_sess_ctx.c
* \author     hejian
* \date       2015/06/03
* \version    FlexBNG V1
* \brief      用户执行命令模式实现
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "dss_incl.h"

#define  DSS_ADMIN_PROMPT  "[ADMIN]"

/* 下面定义每个telnet会话的相关结构*/

/*会话当前的key*/
typedef  struct tagDSS_SESS_CTX_KEY
{
    UINT32  os_pid;
}DSS_SESS_CTX_KEY;

/*会话的上下文环境*/
struct tagDSS_SESS_CTX
{
    SESS_MODE  cur_mode;
    CHAR  cur_mode_str[DS_PROMPT_LEN];
    DSS_SESS_CTX_KEY  cur_mode_key;
};

INT32  dss_sess_ctx_init(DSS_SESS_CTX **ppsess_ctx)
{
    INT32  ret = FALSE;
    
    if (ppsess_ctx == NULL)
    {
        goto  EXIT_LABEL;
    }

    *ppsess_ctx = malloc(sizeof(DSS_SESS_CTX));
    if (*ppsess_ctx == NULL)
    {
        goto  EXIT_LABEL;
    }
    memset(*ppsess_ctx, 0, sizeof(DSS_SESS_CTX));

    (*ppsess_ctx)->cur_mode = SESS_MODE_ADMIN;
    strcpy((*ppsess_ctx)->cur_mode_str, DSS_ADMIN_PROMPT);

    ret = TRUE;
    
EXIT_LABEL:
    return ret;
}

VOID   dss_sess_ctx_uninit(DSS_SESS_CTX **ppsess_ctx)
{
    if (*ppsess_ctx != NULL)
    {
        free(*ppsess_ctx);
        *ppsess_ctx = NULL;
    }

    return;
}

INT32  dss_sess_ctx_set_admin(DSS_SESS_CTX *psess_ctx)
{
    INT32  ret = FALSE;
    
    if (psess_ctx == NULL)
    {
        ds_assert(0);
        goto  EXIT_LABEL;
    }

    psess_ctx->cur_mode = SESS_MODE_ADMIN;
    strncpy(psess_ctx->cur_mode_str, DSS_ADMIN_PROMPT, DS_PROMPT_LEN);

    ret = TRUE;
    
EXIT_LABEL:
    return  ret;
}


INT32  dss_sess_ctx_set_proc(DSS_SESS_CTX *psess_ctx, UINT32 ospid)
{
    INT32  ret = FALSE;
    CHAR  proc_name[DS_PROC_NAME_LEN] = {0};
    
    if (psess_ctx == NULL)
    {
        ds_assert(0);
        goto  EXIT_LABEL;
    }

    psess_ctx->cur_mode = SESS_MODE_PROC;
    psess_ctx->cur_mode_key.os_pid = ospid;

    dss_am_agent_pid2name(ospid, proc_name);
    snprintf(psess_ctx->cur_mode_str, DS_PROMPT_LEN, "[%s-%u]",
             proc_name, ospid);
    ret = TRUE;
    
EXIT_LABEL:
    return  ret;
}


VOID   dss_sess_ctx_get_mode(DSS_SESS_CTX *psess_ctx, SESS_MODE *pmode)
{
    if ((psess_ctx == NULL) || (pmode == NULL))
    {
        ds_assert(0);
        goto  EXIT_LABEL;
    }

    *pmode = psess_ctx->cur_mode;
    
EXIT_LABEL:
    return;
}

VOID  dss_sess_ctx_get_mode_str(DSS_SESS_CTX *psess_ctx, CHAR *mode_str)
{
    if ((psess_ctx == NULL) || (mode_str == NULL))
    {
        ds_assert(0);
        goto  EXIT_LABEL;
    }

    strncpy(mode_str, psess_ctx->cur_mode_str, DS_PROMPT_LEN);
    
EXIT_LABEL:
    return;
}

VOID   dss_sess_ctx_get_ospid(DSS_SESS_CTX *psess_ctx, UINT32 *ospid)
{
    if ((psess_ctx == NULL) || (ospid == NULL))
    {
        ds_assert(0);
        goto  EXIT_LABEL;
    }

    ds_assert(psess_ctx->cur_mode == SESS_MODE_PROC);
    *ospid = psess_ctx->cur_mode_key.os_pid;
    
EXIT_LABEL:
    return;
}

