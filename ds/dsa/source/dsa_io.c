/*****************************************************************************/
/** 
* \file       dsa_io.c
* \author     hejian
* \date       2015/06/08
* \version    FlexBNG V1
* \brief      DSA输出重定向处理
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "dsa_incl.h"

struct DSA_IO_DATA
{
    UINT8  *pbuf_output;
}g_dsa_io_data;


#define DS_WRITE_BUF_LEN  2048

int  DS_PRINTF(const char *fmt, ...)
{
    CHAR  def_buf[2048] = {0};
    CHAR *big_buf = NULL;
    va_list argptr;
    INT32 rtn_len = 0;
    INT32 wrd_len = 0;
    
    va_start(argptr, fmt);

    /**/
    rtn_len = vsnprintf(def_buf, DS_WRITE_BUF_LEN, fmt, argptr);
    if (rtn_len > DS_WRITE_BUF_LEN)
    {
        big_buf = malloc(rtn_len);
        if (big_buf == NULL)
        {
            ds_assert(0);
            va_end(argptr);
            goto EXIT_LABEL;
        }
        
        va_end(argptr);
        va_start(argptr, fmt);
        rtn_len = vsnprintf(big_buf, rtn_len, fmt, argptr);
    }
    else
    {
       big_buf = def_buf;
    }
    va_end(argptr);

    dsa_msg_snd_output((UINT8 *)big_buf, rtn_len);

    if (big_buf != def_buf)
    {
        free(big_buf);
    }

    return wrd_len;
EXIT_LABEL:
    return -1;
}


INT32  dsa_io_init(DS_EVENT_MNG* evt_mng)
{
    g_dsa_io_data.pbuf_output = malloc(DS_MSG_OUTPUT_BUF_LEN);
    if (g_dsa_io_data.pbuf_output == NULL)
    {
        ds_assert(0);
        goto  EXIT_LABEL;
    }

    return  TRUE;
EXIT_LABEL:
    dsa_io_uninit();
    return  FALSE;
}

VOID  dsa_io_uninit(VOID)
{
    if (g_dsa_io_data.pbuf_output != NULL)
    {
        free(g_dsa_io_data.pbuf_output);
    }
    
    return;
}
