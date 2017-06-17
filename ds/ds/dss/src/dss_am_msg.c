/*****************************************************************************/
/** 
* \file       dss_am_msg.c
* \author     hejian
* \date       2015/06/06
* \version    FlexBNG V1
* \brief      Agent Manager Message Process
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "dss_incl.h"

/*Receive buf cache for all agent*/
static UINT8  g_dss_am_rcv_buf[DS_MSG_BUF_LEN] = {0};
static UINT32 g_dss_am_rd_pos = 0;

#if  DSS_CODE("内部接口")
INT32  dss_am_msg_reg_ack_snd(DSS_AM_AGENT *agent, DS_MSG_REG *reg_ack_msg)
{
    INT32  msg_len = 0;
    
    ds_assert(agent);
    ds_assert(reg_ack_msg);

    msg_len = reg_ack_msg->head.len;
    if (msg_len != write(agent->agent_fd, reg_ack_msg, msg_len))
    {
        ds_log_err("Failed to send reg ack msg %d\n", agent->agent_fd);
        goto FAIL_LABEL;
    }

    dss_am_agent_fsm_input(agent, DSS_AGENT_REG_ACK_SND);

    return  TRUE;
FAIL_LABEL:
    return  FALSE;
}

INT32  dss_am_msg_reg_rcv(DSS_AM_AGENT *agent, DS_MSG_REG *reg_msg)
{
    INT32  ret = FALSE;
    
    ds_assert(agent);
    ds_assert(reg_msg);

    dss_am_agent_fsm_input(agent, DSS_AGENT_REG_RCV);

    agent->os_pid = reg_msg->os_pid;
    strncpy(agent->proc_name, reg_msg->proc_name, DS_PROC_NAME_LEN);

    ret = dss_am_msg_reg_ack_snd(agent, reg_msg);

    return  ret;
}

INT32  dss_am_msg_output_rcv(DSS_AM_AGENT *agent, DS_MSG_OUTPUT *output_msg)
{   
    ds_assert(agent);
    ds_assert(output_msg);

    dss_am_agent_fsm_input(agent, DSS_AGENT_OUTPUT_RCV);

    if (output_msg->head.len < sizeof(DS_MSG_OUTPUT))
    {
        ds_log("Invalid output msg length find %u", output_msg->head.len);
        return FALSE;
    }

    /*Output to specific terminate*/
    if (output_msg->sess_index == 0)
    {
        dss_sess_write_all_term(output_msg->output,
            output_msg->head.len - sizeof(DS_MSG_OUTPUT));
    }
    else
    {
        dss_sess_write_term(output_msg->sess_index, output_msg->output,
            output_msg->head.len - sizeof(DS_MSG_OUTPUT));
    }
    ds_log("---%s---", output_msg->output);
    
    return  TRUE;
}

INT32  dss_am_msg_dispatch(DSS_AM_AGENT *agent, UINT8 *pmsg, UINT32 msg_len)
{
    DS_MSG_HEADER  *header_ptr = NULL;
    
    header_ptr = (DS_MSG_HEADER *)g_dss_am_rcv_buf;
    while (msg_len > 0)
    {
        /*rcv incomplete msg, log it and leave process later*/
        if ((header_ptr->len > msg_len) || (msg_len < sizeof(DS_MSG_HEADER)))
        {
            g_dss_am_rd_pos = msg_len;
            memmove(g_dss_am_rcv_buf, header_ptr, msg_len);
            goto  FAIL_LABEL;
        }

        switch (header_ptr->type)
        {
            case DS_MSG_TYPE_REG:
                dss_am_msg_reg_rcv(agent, (DS_MSG_REG *)header_ptr);
                break;
            case DS_MSG_TYPE_OUTPUT:
                dss_am_msg_output_rcv(agent, (DS_MSG_OUTPUT *)header_ptr);
                break;
            default:
                ds_assert(0);
                goto  FAIL_LABEL;
        };
        
        msg_len -= header_ptr->len;
        header_ptr = (DS_MSG_HEADER *)((UINT8 *)header_ptr + header_ptr->len);
    }

    g_dss_am_rd_pos = 0;
    
    return  TRUE;
FAIL_LABEL:
    return  FALSE;
}
#endif

INT32  dss_am_msg_rcv(DSS_AM_AGENT *agent, INT32 fd)
{
    INT32  rcv_len = 0;
    INT32  read_len = 0;

    ds_log("Recv msg from agent %p, %d\n", agent, fd);
    
    while (TRUE)
    {
        read_len = DS_MSG_BUF_LEN - g_dss_am_rd_pos;
        rcv_len = recv(fd, (g_dss_am_rcv_buf + g_dss_am_rd_pos),
                       read_len, 0);
        if (rcv_len == 0)
        {
            ds_log_err("Agent %u terminate normally\n", fd);
            dss_am_agent_del(fd);
            goto FAIL_LABEL;
        }
        else if (rcv_len < 0)
        {
            if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
            {
                ds_log_err("No more data from agent %u this time\n", fd);
            }
            else
            {
                ds_log_err("Agent %u terminate really\n", fd);
                dss_am_agent_del(fd);
            }
            goto FAIL_LABEL;
        }

        dss_am_msg_dispatch(agent, g_dss_am_rcv_buf, rcv_len + g_dss_am_rd_pos);

        /*no need to read any more*/
        if (rcv_len < read_len)
        {
            break;
        }
    }
    
    return  TRUE;

FAIL_LABEL:
    return  FALSE;
}

INT32  dss_am_msg_cmd_snd(DSS_TSESSION  *ts, const CHAR *msg_str)
{
    DSS_AM_AGENT *target_agent = NULL;
    DSS_SESS_CTX *sess_ctx = NULL;
    SESS_MODE cur_mode;
    UINT32  os_pid = 0;
    INT32  sess_index = 0;
    DS_MSG_CMD  *pcmd_msg = NULL;
    INT32  msg_len = 0;

    ds_assert(ts);
    ds_assert(msg_str);
    
    sess_ctx = dss_sess_get_ctx(ts);
    if (sess_ctx == NULL)
    {
        ds_assert(0);
        goto  FAIL_LABEL;
    }
    dss_sess_ctx_get_mode(sess_ctx, &cur_mode);
    if (cur_mode != SESS_MODE_PROC)
    {
        ds_assert(0);
        goto  FAIL_LABEL;
    }
    
    dss_sess_ctx_get_ospid(sess_ctx, &os_pid);
    target_agent = dss_am_agent_find_by_pid(os_pid);
    if (target_agent == NULL)
    {
        ds_log_err("Target process may be dead!\n");
        goto  FAIL_LABEL;
    }

    sess_index = dss_sess_get_idx(ts);

    msg_len = sizeof(DS_MSG_CMD) + strlen(msg_str) + 1;
    pcmd_msg = malloc(msg_len);
    if (pcmd_msg == NULL)
    {
        ds_assert(0);
        goto  FAIL_LABEL;
    }
    memset(pcmd_msg, 0, msg_len);

    pcmd_msg->head.type = DS_MSG_TYPE_CMD;
    pcmd_msg->head.len  = msg_len;
    pcmd_msg->sess_index = sess_index;
    strcpy(pcmd_msg->cmd_str, msg_str);
    
    if (msg_len != write(target_agent->agent_fd, pcmd_msg, msg_len))
    {
        ds_log_err("Failed to send reg ack msg %d\n", target_agent->agent_fd);
        goto FAIL_LABEL;
    }

    free(pcmd_msg);
    return  TRUE;
FAIL_LABEL:
    if (pcmd_msg)
    {
        free(pcmd_msg);
    }
    return  FALSE;
}
