/*****************************************************************************/
/** 
* \file       ds_comm_def.h
* \author     hejian
* \date       2015/06/02
* \version    FlexBNG V1
* \brief      一些通用的定义
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#ifndef  _DS_COMM_DEF_H_
#define  _DS_COMM_DEF_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define  DS_PROMPT_LEN      32
#define  DS_PROC_NAME_LEN   20

/*max number of chars per line we receive from client*/
#define  DS_MAX_CHARS_PER_LINE   256

#define  DS_AM_LISTEN_NAME  "/tmp/Flexbng.ds"

#define  ds_assert(exp)       assert(exp)
#define  ds_log(fmt,...)      printf(fmt, ##__VA_ARGS__)
#define  ds_log_err(fmt, ...) printf("%s "fmt, strerror(errno), ##__VA_ARGS__)
#define  offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/*String match algorithm: ignore the leading blank char.*/
#define DS_STR_MATCH(str, x) \
({ \
    INT32  __cmp = 0; \
    INT32  __ii = 0; \
    INT32  __jj = 0; \
    INT32  __len_str = strlen(str); \
    INT32  __len_x = strlen(x); \
    while ((__ii < __len_str) && (__jj < __len_x)) \
    { \
        __cmp = 1; \
        if (str[__ii] != x[__jj]) \
        { \
            __cmp = 0; \
            break; \
        } \
        __ii++; \
        __jj++; \
    } \
    if ((__ii < __len_str) && str[__ii] != ' ') \
    { \
        __cmp = 0; \
    } \
    __cmp; \
})

#ifdef __cplusplus
}
#endif

#endif
