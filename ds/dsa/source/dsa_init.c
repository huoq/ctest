/*****************************************************************************/
/** 
* \file       dsa_init.c
* \author     hejian
* \date       2015/06/06
* \version    FlexBNG V1
* \brief      DSA模块入口
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "dsa_incl.h"

typedef  struct tagDSA_GLOBAL_DATA
{
    INT32  fd;
    DS_EVENT_MNG *event_mng;
    INT32  cur_sess_index;
}DSA_GLOBAL_DATA;

DSA_GLOBAL_DATA  g_dsa_global_data;

#if  DSA_CODE("内部接口")
VOID  dsa_income_event_handler(INT32  fd)
{
    ds_assert(g_dsa_global_data.fd == fd);

    if (!dsa_msg_rcv(fd))
    {
        ds_event_mng_del_event(g_dsa_global_data.event_mng, g_dsa_global_data.fd,
                           DS_EVENT_IN);
        close(g_dsa_global_data.fd);
        g_dsa_global_data.fd = -1;
        dsa_conn_fsm_input(DSA_CONN_DESTROY);
    }
    
    return;
}

VOID* dsa_connection_thread_handler(VOID *param)
{
    struct  sockaddr_un  un;
    int len = 0;

_TRAY_AGAIN_:
    g_dsa_global_data.fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (g_dsa_global_data.fd == -1)
    {
        ds_log_err("Create socket failed\n");
        sleep(5);
        goto  _TRAY_AGAIN_;
    }

    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, DS_AM_LISTEN_NAME);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(DS_AM_LISTEN_NAME);

    if (connect(g_dsa_global_data.fd, (struct sockaddr *)&un, len) < 0)
    {
        /*ds_log_err("Connect to server failed\n");*/
        close(g_dsa_global_data.fd);
        sleep(5);
        goto  _TRAY_AGAIN_;
    }

    DS_FD_SET_NONBLK(g_dsa_global_data.fd);

    dsa_msg_set_peer_fd(g_dsa_global_data.fd);
    dsa_conn_fsm_input(DSA_CONN_CONNECTED);

    ds_event_mng_add_event(g_dsa_global_data.event_mng, g_dsa_global_data.fd,
                           DS_EVENT_IN, dsa_income_event_handler);
    
    /*There come endless loop~~*/                       
    ds_event_mng_process(g_dsa_global_data.event_mng);

    /*Surely we disconnect from server,try again*/
    ds_assert(g_dsa_global_data.fd == -1);
    sleep(5);
    goto _TRAY_AGAIN_;
    
    return  NULL;
}

INT32  dsa_connection_thread_start(VOID)
{
    INT32  ret = -1;
    pthread_t am_tid;
    
    ret = pthread_create(&am_tid, NULL, dsa_connection_thread_handler, NULL);
    if (ret == -1)
    {
        goto FAIL_LABEL;
    }

    /* detach for resource free */
    pthread_detach(am_tid);

FAIL_LABEL:
    return  ret;
}
#endif

VOID *dsa_init(void)
{    
    /*Init global data*/
    memset(&g_dsa_global_data, 0, sizeof(DSA_GLOBAL_DATA));
    g_dsa_global_data.event_mng = ds_event_mng_create(10);
    if (g_dsa_global_data.event_mng == NULL)
    {
        goto  EXIT_LABEL;
    }

    /*Init parser*/
    ds_comm_parse_init();

    /*Init elf parse*/
    dsa_elf_init();
    
    /*Init cmd module*/
    dsa_cmd_init();

    /*Init msg module*/
    dsa_msg_init();
    
    /*Init io module*/
    dsa_io_init(g_dsa_global_data.event_mng);

    /*Start connect to dss*/
    dsa_connection_thread_start();

    return (VOID *)(&g_dsa_global_data);

EXIT_LABEL:
    return NULL;
}
