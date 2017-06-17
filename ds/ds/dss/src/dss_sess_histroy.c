/*****************************************************************************/
/** 
* \file       dss_sess_histroy.h
* \author     hejian
* \date       2015/06/10
* \version    FlexBNG V1
* \brief      上下键切换历史命令管理实现
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "dss_incl.h"

typedef struct tagDSS_HISTROY_CMD
{
    CIRCLEQ_ENTRY(tagDSS_HISTROY_CMD)  cmd_entry;
    CHAR  *cmd_string;
}DSS_HISTROY_CMD;


struct tagDSS_SESS_HISTROY
{
    CIRCLEQ_HEAD(tag_cmd_head, tagDSS_HISTROY_CMD)  cmd_head;
    UINT32  max_num;
    UINT32  cur_num;
    DSS_HISTROY_CMD  *pos_entry; /*前后遍历的游标*/
};

#define  DSS_HISTROY_CMD_DEL(HEAD, ELE) \
do \
{ \
    CIRCLEQ_REMOVE(&(HEAD), (ELE), cmd_entry); \
    ds_assert((ELE)->cmd_string); \
    free((ELE)->cmd_string); \
    free((ELE)); \
}while(0)


DSS_SESS_HISTROY*  dss_histroy_init(UINT32 max_cmd_num)
{
    DSS_SESS_HISTROY  *sess_histroy = NULL;

    sess_histroy = malloc(sizeof(DSS_SESS_HISTROY));
    if (sess_histroy == NULL)
    {
        ds_assert(0);
        goto  EXIT_LABEL;
    }
    memset(sess_histroy, 0, sizeof(DSS_SESS_HISTROY));

    sess_histroy->cur_num = 0;
    sess_histroy->max_num = max_cmd_num;
    
    CIRCLEQ_INIT(&(sess_histroy->cmd_head));
    sess_histroy->pos_entry = NULL;

EXIT_LABEL:
    return  sess_histroy;
}

INT32  dss_histroy_insert(DSS_SESS_HISTROY *sess_his, CHAR *cmd_str)
{
    DSS_HISTROY_CMD  *ptmp_cmd = NULL;
    CHAR *new_str = NULL;
    
    ds_assert(sess_his);
    ds_assert(cmd_str);

    if (strlen(cmd_str) == 0)
    {
        goto  FAIL_LABEL;
    }
    
    ptmp_cmd = malloc(sizeof(DSS_HISTROY_CMD));
    if (ptmp_cmd == NULL)
    {
        goto  FAIL_LABEL;
    }
    memset(ptmp_cmd, 0, sizeof(DSS_HISTROY_CMD));
    
    new_str = malloc(strlen(cmd_str) + 1);
    if (new_str == NULL)
    {
        goto  FAIL_LABEL;
    }
    memset(new_str, 0, strlen(cmd_str) + 1);
    memcpy(new_str, cmd_str, strlen(cmd_str));

    ptmp_cmd->cmd_string = new_str;
    CIRCLEQ_INSERT_HEAD(&(sess_his->cmd_head), ptmp_cmd, cmd_entry);

    ds_assert(sess_his->cur_num <= sess_his->max_num);
    sess_his->cur_num++;

    /*delete old entry*/
    if (sess_his->cur_num >= sess_his->max_num)
    {
        sess_his->cur_num = sess_his->max_num;
        ptmp_cmd = CIRCLEQ_LAST(&(sess_his->cmd_head));
        DSS_HISTROY_CMD_DEL(sess_his->cmd_head, ptmp_cmd);
    }

    sess_his->pos_entry = NULL;
    
    return  TRUE;
FAIL_LABEL:
    if (new_str != NULL)
    {
        free(new_str);
    }
    if (ptmp_cmd != NULL)
    {
        free(ptmp_cmd);
    }
    
    return  FALSE;
}


INT32  dss_histroy_prev(DSS_SESS_HISTROY *sess_his, CHAR **ppcmd_str)
{
    DSS_HISTROY_CMD  *ptmp_cmd = NULL;
    
    ds_assert(sess_his);
    ds_assert(ppcmd_str);

    if (CIRCLEQ_EMPTY(&(sess_his->cmd_head)))
    {
        goto  FAIL_LABEL;
    }
    
    if (sess_his->pos_entry == NULL)
    {
        ptmp_cmd = CIRCLEQ_FIRST(&(sess_his->cmd_head));
    }
    else
    {
        ptmp_cmd = CIRCLEQ_NEXT(sess_his->pos_entry, cmd_entry);
        if (ptmp_cmd == (VOID *)&(sess_his->cmd_head))
        {
            goto  FAIL_LABEL;
        }
    }

    ds_assert(ptmp_cmd);
    if (ptmp_cmd == NULL)
    {
        goto  FAIL_LABEL;
    }
    *ppcmd_str = ptmp_cmd->cmd_string;

    sess_his->pos_entry = ptmp_cmd;
    
    return  TRUE;
FAIL_LABEL:
    return  FALSE;
}


INT32  dss_histroy_next(DSS_SESS_HISTROY *sess_his, CHAR **ppcmd_str)
{
    DSS_HISTROY_CMD  *ptmp_cmd = NULL;
    
    ds_assert(sess_his);
    ds_assert(ppcmd_str);

    if (CIRCLEQ_EMPTY(&(sess_his->cmd_head)))
    {
        goto  FAIL_LABEL;
    }
    
    if (sess_his->pos_entry == NULL)
    {
        ptmp_cmd = CIRCLEQ_LAST(&(sess_his->cmd_head));
    }
    else
    {
        ptmp_cmd = CIRCLEQ_PREV(sess_his->pos_entry, cmd_entry);
        if (ptmp_cmd == (VOID *)&(sess_his->cmd_head))
        {
            goto  FAIL_LABEL;
        }
    }

    ds_assert(ptmp_cmd);
    if (ptmp_cmd == NULL)
    {
        goto  FAIL_LABEL;
    }
    *ppcmd_str = ptmp_cmd->cmd_string;

    sess_his->pos_entry = ptmp_cmd;
    
    return  TRUE;
FAIL_LABEL:
    return  FALSE;
}


VOID   dss_histroy_uninit(DSS_SESS_HISTROY *sess_his)
{
    DSS_HISTROY_CMD  *ptmp_cmd = NULL;
    
    ds_assert(sess_his);

    while (!CIRCLEQ_EMPTY(&(sess_his->cmd_head)))
    {
        ptmp_cmd = CIRCLEQ_FIRST(&(sess_his->cmd_head));
        DSS_HISTROY_CMD_DEL(sess_his->cmd_head, ptmp_cmd);
    }
    
    return ;
}
