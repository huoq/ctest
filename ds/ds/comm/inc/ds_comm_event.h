/*****************************************************************************/
/** 
* \file       ds_comm_event.h
* \author     hejian
* \date       2015/06/05
* \version    FlexBNG V1
* \brief      封装的epoll io事件处理头文件
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/

#ifndef  _DS_COMM_EVENT_H_
#define  _DS_COMM_EVENT_H_

#ifdef  __cplusplus
extern "C"
{
#endif

struct tagDS_EVENT_MNG;
typedef struct tagDS_EVENT_MNG  DS_EVENT_MNG;

typedef VOID  (*DS_EVENT_HANDLER)(INT32  fd);

typedef enum
{
    DS_EVENT_IN = 1,
    DS_EVENT_OUT
}DS_EVENT_TYPE;

DS_EVENT_MNG*  ds_event_mng_create(UINT32  max_num);
VOID  ds_event_mng_destroy(DS_EVENT_MNG* event_mng);
INT32  ds_event_mng_add_event(DS_EVENT_MNG* event_mng, INT32  fd,
                              DS_EVENT_TYPE  event, DS_EVENT_HANDLER  handler);
INT32  ds_event_mng_del_event(DS_EVENT_MNG* event_mng, INT32  fd,
                              DS_EVENT_TYPE  event);
INT32          ds_event_mng_process(DS_EVENT_MNG *event_mng);


#ifdef  __cplusplus
}
#endif
#endif
