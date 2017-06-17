/*****************************************************************************/
/** 
* \file       ds_comm_cmd.h
* \author     hejian
* \date       2015/06/05
* \version    FlexBNG V1
* \brief      ͨ���������ṹͷ�ļ�
* \note       Copyright (c) 2000-2020  ����˹��Ϣ�Ƽ��ɷ����޹�˾
* \remarks    �޸���־
******************************************************************************/
#ifndef  _DS_COMM_CMD_H_
#define  _DS_COMM_CMD_H_

#ifdef  __cplusplus
extern "C"
{
#endif

typedef VOID  (*DS_CMD_HANDLER)(CHAR *cmd_str, VOID *ctx);

typedef  struct tagDS_CMD_ENTRY
{
    CHAR  *cmd_str;
    CHAR  *alias_str;
    CHAR  *help_str;
    DS_CMD_HANDLER  cmd_handler;
    TAILQ_ENTRY(tagDS_CMD_ENTRY) cmd_entry;
}DS_CMD_ENTRY;

typedef  struct tagDS_CMD_LIST
{
    TAILQ_HEAD(tagDS_CMD_HEAD, tagDS_CMD_ENTRY) cmd_head;
}DS_CMD_LIST;

INT32           ds_cmd_install(DS_CMD_LIST *pcmd_list, DS_CMD_ENTRY *pcmd_entry, 
                               UINT32 count);
                               
DS_CMD_HANDLER  ds_cmd_handler_find(DS_CMD_LIST *pcmd_list,
                                    const CHAR  *cmd_str);

VOID            ds_cmd_list_print(DS_CMD_LIST *pcmd_list);

#ifdef  __cplusplus
}
#endif
#endif
