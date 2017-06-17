/*****************************************************************************/
/** 
* \file       dsa_msg.c
* \author     hejian
* \date       2015/06/08
* \version    FlexBNG V1
* \brief      处理从DSS过来的消息
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "dsa_incl.h"

typedef  enum
{
    DSA_STATE_INIT = 1,    /*Init State*/
    DSA_STATE_CONNECTED,
    DSA_STATE_ACTIVE,  /*After Reg ack msg has rcv from dss*/
    DSA_STATE_CMD_RCV
}DSA_CONN_STATE;

struct DSA_MSG_DATA
{
    UINT8  *prcv_buf;
    INT32  peer_fd;
    INT32  cur_sess_index;
    DSA_CONN_STATE cur_state;
}g_dsa_msg_data;

#if  DSA_CODE("内部接口")
INT32  dsa_msg_reg_snd(VOID)
{
    DS_MSG_REG  msg_reg;
    UINT32 len = 0;
    
    ds_assert(g_dsa_msg_data.peer_fd != -1);
    memset(&msg_reg, 0, sizeof(DS_MSG_REG));

    len = sizeof(DS_MSG_REG);
    msg_reg.head.type = DS_MSG_TYPE_REG;
    msg_reg.head.len  = len;
    msg_reg.os_pid = getpid();
    prctl(PR_GET_NAME, msg_reg.proc_name);

    if (len != write(g_dsa_msg_data.peer_fd, &msg_reg, len))
    {
        ds_log_err("Failed to send reg msg\n");
        return  FALSE;
    }
    
    return  TRUE;
}

INT32  dsa_msg_reg_rcv(INT32 fd, DS_MSG_REG *reg_msg)
{
    ds_assert(reg_msg);

    dsa_conn_fsm_input(DSA_CONN_REG_ACK_RCV);

    return  TRUE;
}

INT32  dsa_msg_cmd_rcv(INT32 fd, DS_MSG_CMD *cmd_msg)
{
    INT32  ret = FALSE;
    
    ds_assert(cmd_msg);

    dsa_conn_fsm_input(DSA_CONN_CMD_MSG_RCV);

    /*Save the current session index*/
    g_dsa_msg_data.cur_sess_index = cmd_msg->sess_index;
    
    /**/
    dsa_cmd_dispatch(cmd_msg->cmd_str);

    return  ret;
}


INT32  dsa_msg_dispatch(INT32 fd, UINT8 *msg_buf, UINT32 msg_len)
{
    DS_MSG_HEADER  *header_ptr = NULL;
    
    header_ptr = (DS_MSG_HEADER *)msg_buf;
    while (msg_len > 0)
    {
        if (header_ptr->len > msg_len)
        {
            ds_assert(0);
            goto  FAIL_LABEL;
        }

        switch (header_ptr->type)
        {
            case DS_MSG_TYPE_REG:
                /*reg ack from dss*/
                dsa_msg_reg_rcv(fd, (DS_MSG_REG *)header_ptr);
                break;
            case DS_MSG_TYPE_CMD:
                dsa_msg_cmd_rcv(fd, (DS_MSG_CMD *)header_ptr);
                break;
            default:
                ds_assert(0);
                goto  FAIL_LABEL;
        };
        
        msg_len -= header_ptr->len;
        header_ptr = (DS_MSG_HEADER *)((UINT8 *)header_ptr + header_ptr->len);
    }

    return  TRUE;
FAIL_LABEL:
    return  FALSE;
}
#endif

INT32  dsa_msg_snd_output(UINT8 *out_buf, INT32 buf_len)
{
    INT32  ret = FALSE;
    DS_MSG_OUTPUT  *pmsg_output = NULL;
    INT32 len = 0;
    INT32 wrd_len = 0;

    ds_assert(out_buf);
    if (g_dsa_msg_data.peer_fd == -1)
    {
        /*here maybe dsa not connected*/
        goto  FAIL_LABEL;
    }

    len = sizeof(DS_MSG_OUTPUT) + buf_len;
    pmsg_output = malloc(len);
    if (pmsg_output == NULL)
    {
        ds_assert(0);
        goto  FAIL_LABEL;
    }
    memset(pmsg_output, 0, len);

    pmsg_output->head.type = DS_MSG_TYPE_OUTPUT;
    pmsg_output->head.len  = len;
    pmsg_output->sess_index = g_dsa_msg_data.cur_sess_index;
    memcpy(pmsg_output->output, out_buf, buf_len);

    while (1)
    {
        wrd_len = write(g_dsa_msg_data.peer_fd, pmsg_output, len);
        if (wrd_len < 0)
        {
             if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
             {
                 usleep(1000);
                 continue;
             }
             goto FAIL_LABEL;
        }

        break;
    }
    
    ret = TRUE;

FAIL_LABEL:
    if (pmsg_output != NULL)
    {
        free(pmsg_output);
    }
    return  ret;
}


INT32   dsa_msg_rcv(INT32 fd)
{
    INT32  rcv_len = 0;

    ds_log("Recv msg from dss %d\n", fd);
    
    while (TRUE)
    {
        rcv_len = recv(fd, g_dsa_msg_data.prcv_buf, DS_MSG_BUF_LEN, 0);
        if (rcv_len == 0)
        {
            ds_log_err("Connection %d terminate normally\n", fd);
            goto FAIL_LABEL;
        }
        else if (rcv_len < 0)
        {
            if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
            {
                /*ds_log_err("No more data from dss %u this time\n", fd);*/
                break;
            }
            else
            {
                ds_log_err("Connection %d terminate really\n", fd);
                goto FAIL_LABEL;
            }
        }

        /*目前使用unix套接字进行dss和各个dsa之间的通信，
          不需要考虑可靠性、粘包的情况*/
        dsa_msg_dispatch(fd, g_dsa_msg_data.prcv_buf, rcv_len);
    }
    
    return  TRUE;

FAIL_LABEL:
    return  FALSE;
}

INT32  dsa_conn_fsm_input(DSA_CONN_EVENT fsm_event)
{
    switch (fsm_event)
    {
        case  DSA_CONN_CONNECTED:
            if (g_dsa_msg_data.cur_state != DSA_STATE_INIT)
            {
                ds_assert(0);
                goto  FAIL_LABEL;
            }
            
            /*Send reg msg*/
            dsa_msg_reg_snd();

            /*Then trans to connected state*/
            g_dsa_msg_data.cur_state = DSA_STATE_CONNECTED;
            break;
        case  DSA_CONN_REG_ACK_RCV:
            if (g_dsa_msg_data.cur_state != DSA_STATE_CONNECTED)
            {
                ds_assert(0);
                goto  FAIL_LABEL;
            }
            g_dsa_msg_data.cur_state = DSA_STATE_ACTIVE;
            break;
        case  DSA_CONN_CMD_MSG_RCV:
            if ((g_dsa_msg_data.cur_state != DSA_STATE_ACTIVE)
                && (g_dsa_msg_data.cur_state != DSA_STATE_CMD_RCV))
            {
                ds_assert(0);
                goto  FAIL_LABEL;
            }
            break;
        case  DSA_CONN_DESTROY:
            g_dsa_msg_data.cur_state = DSA_STATE_INIT;
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

VOID  dsa_msg_set_peer_fd(INT32  fd)
{
    g_dsa_msg_data.peer_fd = fd;

    return;
}

INT32  dsa_msg_init(VOID)
{
    memset(&g_dsa_msg_data, 0, sizeof(struct DSA_MSG_DATA));
    g_dsa_msg_data.prcv_buf = malloc(DS_MSG_BUF_LEN);
    if (g_dsa_msg_data.prcv_buf == NULL)
    {
        ds_assert(0);
        goto  FAIL_LABEL;
    }

    g_dsa_msg_data.peer_fd = -1;

    g_dsa_msg_data.cur_state = DSA_STATE_INIT;
    
    return  TRUE;
FAIL_LABEL:
    dsa_msg_uninit();
    return  FALSE;
}

VOID  dsa_msg_uninit(VOID)
{
    if (g_dsa_msg_data.prcv_buf != NULL)
    {
        free(g_dsa_msg_data.prcv_buf);
    }

    return;
}
