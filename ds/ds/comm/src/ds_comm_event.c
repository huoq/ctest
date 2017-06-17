/*****************************************************************************/
/** 
* \file       ds_comm_event.h
* \author     hejian
* \date       2015/06/06
* \version    FlexBNG V1
* \brief      封装的epoll io事件处理
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/

#include "ds_comm_incl.h"


typedef  struct tagDS_EVENT
{
    RB_ENTRY(tagDS_EVENT) event_entry;
    INT32 fd;
    DS_EVENT_HANDLER handler;
}DS_EVENT;

struct tagDS_EVENT_MNG
{
    INT32  epoll_fd;
    struct epoll_event *pevents;
    UINT32 max_count;
    RB_HEAD(DS_EVENT_TREE, tagDS_EVENT) event_tree;
};

#if  DS_CODE("内部接口")
#define  DS_EVENT_TRANS(EVENT) \
({ \
    INT32 __epoll_event = EPOLLIN; \
    if (EVENT == DS_EVENT_IN) \
    { \
        __epoll_event = EPOLLIN; \
        __epoll_event |= EPOLLET; \
    } \
    else if (EVENT == DS_EVENT_OUT) \
    { \
        __epoll_event = EPOLLOUT; \
    } \
    else \
    { \
        ds_assert(0); \
    } \
    __epoll_event; \
})

INT32  ds_event_cmp(DS_EVENT *ea, DS_EVENT *eb)
{
    INT32  cmp = 0;
    
    ds_assert(ea);
    ds_assert(eb);

    if (ea->fd > eb->fd)
    {
        cmp = 1;
    }
    else if (ea->fd < eb->fd)
    {
        cmp = -1;
    }
    
    return cmp;
}

RB_GENERATE_STATIC(DS_EVENT_TREE, tagDS_EVENT, event_entry, ds_event_cmp);

#define  _DS_EVENT_DEL(HEAD, ELE) \
do \
{ \
    RB_REMOVE(DS_EVENT_TREE, &(HEAD), ELE); \
    free(ELE); \
}while (0)

#endif

DS_EVENT_MNG*  ds_event_mng_create(UINT32  max_num)
{
    DS_EVENT_MNG *event_mng = NULL;
    
    event_mng = malloc(sizeof(DS_EVENT_MNG));
    if (event_mng == NULL)
    {
        ds_assert(0);
        goto  FAIL_LABEL;
    }
    memset(event_mng, 0, sizeof(DS_EVENT_MNG));

    event_mng->epoll_fd = epoll_create(max_num);
    if (event_mng->epoll_fd == -1)
    {
        ds_log_err("Create epoll fd failed\n");
        goto  FAIL_LABEL;
    }

    event_mng->max_count = max_num;
    event_mng->pevents = malloc(sizeof(struct epoll_event) * max_num);
    if (event_mng->pevents == NULL)
    {
        ds_assert(0);
        goto  FAIL_LABEL;
    }
    
    return  event_mng;
FAIL_LABEL:
    if (event_mng != NULL)
    {
        ds_event_mng_destroy(event_mng);
    }
    return  NULL;
}

VOID  ds_event_mng_destroy(DS_EVENT_MNG* event_mng)
{
    DS_EVENT *cur_event = NULL;
    DS_EVENT *tmp_event = NULL;
    
    ds_assert(event_mng);

    close(event_mng->epoll_fd);
    free(event_mng->pevents);

    RB_FOREACH_SAFE(cur_event, DS_EVENT_TREE, &(event_mng->event_tree), tmp_event)
    {
        _DS_EVENT_DEL(event_mng->event_tree, cur_event);
    }
    
    free(event_mng);

    return;
}

INT32  ds_event_mng_add_event(DS_EVENT_MNG* event_mng, INT32  fd,
                              DS_EVENT_TYPE  event, DS_EVENT_HANDLER  handler)
{
    DS_EVENT  *new_event_entry = NULL;
    struct epoll_event ee;
    
    ds_assert(event_mng);

    
    new_event_entry = malloc(sizeof(DS_EVENT));
    if (new_event_entry == NULL)
    {
       ds_assert(0);
       goto  FAIL_LABEL;
    }
    memset(new_event_entry, 0, sizeof(DS_EVENT));
    
    new_event_entry->fd = fd;
    new_event_entry->handler = handler;
    if (RB_INSERT(DS_EVENT_TREE, &(event_mng->event_tree), new_event_entry))
    {
        /*Duplicate entry,error*/
        ds_assert(0);
        goto  FAIL_LABEL;
    }
    
    memset(&ee, 0, sizeof(struct epoll_event));
    ee.events = DS_EVENT_TRANS(event);
    ee.data.fd = fd;
    epoll_ctl(event_mng->epoll_fd, EPOLL_CTL_ADD, fd, &ee);

    
    return  TRUE;
FAIL_LABEL:
    if (new_event_entry != NULL)
    {
        _DS_EVENT_DEL(event_mng->event_tree, new_event_entry);
    }
    return  FALSE;

}

INT32  ds_event_mng_del_event(DS_EVENT_MNG* event_mng, INT32  fd,
                              DS_EVENT_TYPE  event)
{
    struct epoll_event ee;
    DS_EVENT  find_key;
    DS_EVENT  *del_event_entry = NULL;
    
    ds_assert(event_mng);

    /*find*/
    memset(&find_key, 0, sizeof(DS_EVENT));
    find_key.fd = fd;
    del_event_entry = RB_FIND(DS_EVENT_TREE, &(event_mng->event_tree), &find_key);
    if (del_event_entry == NULL)
    {
        ds_assert(0);
        return  FALSE;
    }

    /*del from epoll*/
    memset(&ee, 0, sizeof(struct epoll_event));
    ee.events = DS_EVENT_TRANS(event);
    ee.data.fd = fd;
    epoll_ctl(event_mng->epoll_fd, EPOLL_CTL_DEL, fd, &ee);

    /*free finally*/
    RB_REMOVE(DS_EVENT_TREE, &(event_mng->event_tree), del_event_entry);
    free(del_event_entry);
    
    return  TRUE;
}

INT32  ds_event_mng_process(DS_EVENT_MNG *event_mng)
{
    INT32  nfds = 0;
    INT32  ii = 0;
    DS_EVENT_HANDLER  fd_handler = NULL;
    DS_EVENT  event_key;
    DS_EVENT  *find_event = NULL;
    
    ds_assert(event_mng);
    memset(&event_key, 0, sizeof(DS_EVENT));

    while (!RB_EMPTY(&(event_mng->event_tree)))
    {
        nfds = epoll_wait(event_mng->epoll_fd, event_mng->pevents,
                          event_mng->max_count, -1);
        if (nfds > 0)
        {
            for (ii = 0; ii < nfds; ii++)
            {
                event_key.fd = event_mng->pevents[ii].data.fd;
                find_event = (DS_EVENT *)RB_FIND(DS_EVENT_TREE, &(event_mng->event_tree),
                                     &event_key);
                if (find_event == NULL)
                {
                    ds_assert(0);
                    return FALSE;
                }
                
                fd_handler = find_event->handler;
                if (fd_handler == NULL)
                {
                    ds_assert(0);
                    return FALSE;
                }

                ds_assert(event_key.fd == find_event->fd);
                fd_handler(event_key.fd);
            }
        }
        else
        {
            /*epoll_wait may return by siginal, if so,then return*/
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                ds_log_err("Epoll_wait return failed\n");
                ds_assert(0);
                return FALSE;
            }
        }
    }
    return  TRUE;
}
