/*****************************************************************************/
/** 
* \file       ds_comm_util.c
* \author     hejian
* \date       2015/06/06
* \version    FlexBNG V1
* \brief      几个常用的函数实现
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "ds_comm_incl.h"

#define  DS_STR_SKIP_BLK(STR) \
({ \
    INT32  __i = 0; \
    CHAR  *next_str = NULL; \
    while (((STR[__i] == ' ') || (STR[__i] == '\t')) \
      && (STR[__i] != '\0')) \
    { \
        __i++; \
    } \
    next_str = STR + __i; \
    next_str; \
})

#define  DS_STR_SKIP_NONBLK(STR) \
({ \
    INT32  __i = 0; \
    CHAR  *next_str = NULL; \
    while (((STR[__i] != ' ') && (STR[__i] != '\t')) \
      && (STR[__i] != '\0')) \
    { \
        __i++; \
    } \
    next_str = STR + __i; \
    next_str; \
})


INT32  ds_util_arg_get(CHAR *in_str, INT32 index, CHAR *arg_str, INT32 arg_len)
{
    CHAR  *word_start = NULL;
    CHAR  *word_end = NULL;
    INT32  cur_index = 0;
    INT32  real_len = 0;
    
    if ((in_str == NULL) || (arg_str == NULL) || (index < 0) || (arg_len < 0))
    {
        ds_assert(0);
        goto  FAIL_LABEL;
    }

    while (cur_index < index)
    {
        /*Skip blank char*/
        word_start = DS_STR_SKIP_BLK(in_str);
        if (*word_start == '\0')
        {
            break;
        }

        /*Skip non blank char*/
        word_end = DS_STR_SKIP_NONBLK(word_start);

        in_str = word_end;
        cur_index++;
    }

    if (cur_index == index)
    {
        real_len = word_end - word_start + 1;
        if (arg_len < real_len)
        {
            goto FAIL_LABEL;
        }
        memcpy(arg_str, word_start, real_len);
        arg_str[real_len] = '\0';
    }
    return  TRUE;

FAIL_LABEL:
     return  FALSE;
}

CHAR*  ds_comm_trim(CHAR *in_buf)
{
    CHAR* rtn_buf = NULL;

    rtn_buf = DS_STR_SKIP_BLK(in_buf);
    in_buf[strlen(in_buf) - 1] = '\0';

    return  rtn_buf;
}
