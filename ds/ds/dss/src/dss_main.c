/*****************************************************************************/
/** 
* \file       dss_main.c
* \author     hejian
* \date       2015/06/02
* \version    FlexBNG V1
* \brief      DSS Main Program Entry
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "dss_incl.h"

INT32  dss_daemonize(VOID)
{
    pid_t pid;
    int fd;

    pid = fork();

    /* In case of fork is error. */
    if (pid < 0)
    {
        perror ("fork");
        return FALSE;
    }

    /* In case of this is parent process. */
    if (pid != 0)
    {
        exit (0);
    }

    /* Become session leader and get pid. */
    pid = setsid();

    if (pid < -1)
    {
        perror ("setsid");
        return FALSE;
    }

    /* Change directory to root. */
    chdir ("/");

    fd = open ("/dev/null", O_RDWR, 0);
    if (fd != -1)
    {
        dup2 (fd, STDIN_FILENO);
        dup2 (fd, STDOUT_FILENO);
        dup2 (fd, STDERR_FILENO);
        if (fd > 2)
        {
          close (fd);
        }
    }

    umask (0027);

    return 0;
}

INT32 main(VOID)
{
    /*Running as daemonize process*/
    dss_daemonize();
    
    /*Init parser*/
    ds_comm_parse_init();

    /*Init agent manager*/
    dss_am_init();

    /*Init cmd dispatch*/
    dss_cmd_init();

    /*The dsa in dss's process*/
    dsa_init();
    
    /*Then start telnetd server*/
    dss_telnetd_start();
    
    return 0;
}
