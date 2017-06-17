/*****************************************************************************/
/** 
* \file       ds_comm_cmd.c
* \author     hejian
* \date       2015/06/05
* \version    FlexBNG V1
* \brief      通用的命令管理
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "ds_comm_incl.h"

#define  DS_CMD_ENTRY_DEL(HEAD, ELE) \
do \
{ \
    TAILQ_REMOVE(&(HEAD), (ELE), cmd_entry); \
    free((ELE)); \
}while(0)


#if  DS_CODE("内部接口")
INT32  __ds_cmd_install(DS_CMD_LIST *pcmd_list, DS_CMD_ENTRY *pcmd_entry)
{
    DS_CMD_ENTRY *new_entry = NULL;

    ds_assert(pcmd_list);
    ds_assert(pcmd_entry);
    new_entry = malloc(sizeof(DS_CMD_ENTRY));
    if (new_entry == NULL)
    {
        goto  FAIL_LABEL;
    }
    memset(new_entry, 0, sizeof(DS_CMD_ENTRY));

    new_entry->cmd_str = pcmd_entry->cmd_str;
    new_entry->alias_str = pcmd_entry->alias_str;
    new_entry->help_str = pcmd_entry->help_str;
    new_entry->cmd_handler = pcmd_entry->cmd_handler;

    TAILQ_INSERT_TAIL(&(pcmd_list->cmd_head), new_entry, cmd_entry);
    
    return  TRUE;
FAIL_LABEL:
    return  FALSE;
}

VOID  __ds_cmd_list_destroy(DS_CMD_LIST *pcmd_list)
{
    DS_CMD_ENTRY  *pcur_entry = NULL;
    DS_CMD_ENTRY  *ptmp_entry = NULL;
    
    ds_assert(pcmd_list);

    TAILQ_FOREACH_SAFE(pcur_entry, &(pcmd_list->cmd_head), cmd_entry, ptmp_entry)
    {
        DS_CMD_ENTRY_DEL(pcmd_list->cmd_head, pcur_entry);
    }
    
    return;
}
#endif

INT32           ds_cmd_install(DS_CMD_LIST *pcmd_list, DS_CMD_ENTRY *pcmd_entry, 
                               UINT32 count)
{
    INT32  ret = FALSE;
    DS_CMD_ENTRY  *cur_cmd_entry = NULL;
    UINT32  i = 0;

    cur_cmd_entry = pcmd_entry;
    for (i = 0; i < count; i++)
    {
        ret = __ds_cmd_install(pcmd_list, cur_cmd_entry);
        if (!ret)
        {
            ds_assert(0);
            goto  FAIL_LABEL;
        }
        cur_cmd_entry++;
    }

    return  TRUE;
FAIL_LABEL:
    __ds_cmd_list_destroy(pcmd_list);
    return  FALSE;
}

DS_CMD_HANDLER  ds_cmd_handler_find(DS_CMD_LIST *pcmd_list,
                                    const CHAR  *cmd_str)
{
    DS_CMD_ENTRY  *pcur_entry = NULL;
    DS_CMD_HANDLER  rtn_handler = NULL;

    ds_assert(pcmd_list);
    ds_assert(cmd_str);

    TAILQ_FOREACH(pcur_entry, &(pcmd_list->cmd_head), cmd_entry)
    {
        /*Compare with command and alias command*/
        if (DS_STR_MATCH(cmd_str, pcur_entry->cmd_str))
        {
            rtn_handler = pcur_entry->cmd_handler;
            break;
        }

        if (DS_STR_MATCH(cmd_str, pcur_entry->alias_str))
        {
            rtn_handler = pcur_entry->cmd_handler;
            break;
        }
    }

    return  rtn_handler;
}

VOID  ds_cmd_list_print(DS_CMD_LIST *pcmd_list)
{
    DS_CMD_ENTRY *pcur_entry = NULL;
    
    ds_assert(pcmd_list);
    
    DS_PRINTF("\ncommand        alias        desc\n");
    DS_PRINTF("-------------------------------------\n");
    
    TAILQ_FOREACH(pcur_entry, &(pcmd_list->cmd_head), cmd_entry)
    {
        DS_PRINTF("%-15s%-13s%s\n",
            pcur_entry->cmd_str,
            pcur_entry->alias_str,
            pcur_entry->help_str);
    }
    DS_PRINTF("-------------------------------------\n");
    
    return;
}
