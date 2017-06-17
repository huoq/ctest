/*****************************************************************************/
/** 
* \file       dss_sess_histroy.h
* \author     hejian
* \date       2015/06/02
* \version    FlexBNG V1
* \brief      telnet服务器实现
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "dss_incl.h"

#define DS_TELNETD_PORT  8888  /*telnet server listen on this port*/
#define DS_TELNETD_MAX_SESS_NUM  10  /*max session number support*/

const CHAR  *banner_str = "\n---------------------------------------------\n"\
        "Welcome to Flex-bng Debugging Shell\n"\
        "---------------------------------------------\n\n";


/* Structure that describes a session */
struct tsession {
    struct tsession *next;
    INT32 sockfd_read, sockfd_write;
    INT32 ptym, ptys;
    INT32 rdidx1, wridx1, size1;
    INT32 rdidx2, wridx2, size2;
    pthread_t thread_id;
    INT32 sess_index;
    DSS_SESS_CTX  *sess_ctx_env;
    DSS_SESS_HISTROY* his_cmd[SESS_MODE_MAX];
};

/*telnet server global data struct*/
struct DSS_TELNETD_GLOBAL {
    struct tsession *sessions;
    int maxfd;
    INT32  total_sess_num; /*当前总共的会话话数目*/
    INT32  cur_sess_index; /*最后一个会话的index*/
}g_dss_sess_mng = 
{
    .sessions = NULL,
    .maxfd = -1,
    .total_sess_num = 0,
    .cur_sess_index = 0
};

enum { BUFSIZE = (4 * 1024 - sizeof(struct tsession)) / 2 };


#if  DSS_CODE("移植的老函数")
int safe_write(int fd, const void *buf, int count)
{
    int n;

    do {
        n = write(fd, buf, count);
    } while ((n < 0) && (errno == EINTR));

    return n;
}

int safe_read(int fd, void *buf, int count)
{
    int n;

    do {
        n = read(fd, buf, count);
    } while ((n < 0) && (errno == EINTR));

    return n;
}

/*
   Remove all IAC's from buf1 (received IACs are ignored and must be removed
   so as to not be interpreted by the terminal).  Make an uninterrupted
   string of characters fit for the terminal.  Do this by packing
   all characters meant for the terminal sequentially towards the end of buf.

   Return a pointer to the beginning of the characters meant for the terminal
   and make *num_totty the number of characters that should be sent to
   the terminal.

   Note - if an IAC (3 byte quantity) starts before (bf + len) but extends
   past (bf + len) then that IAC will be left unprocessed and *processed
   will be less than len.

   CR-LF ->'s CR mapping is also done here, for convenience.

   NB: may fail to remove iacs which wrap around buffer!
 */
static unsigned char *
remove_iacs(struct tsession *ts, int *pnum_totty)
{
    unsigned char *ptr0 = TS_BUF1 + ts->wridx1;
    unsigned char *ptr = ptr0;
    unsigned char *totty = ptr;
    unsigned char *end = ptr + MIN(BUFSIZE - ts->wridx1, ts->size1);
    int num_totty;

    while (ptr < end) {
        if (*ptr != IAC) {
            char c = *ptr;

            *totty++ = c;
            ptr++;
            /* We map \r\n ==> \r for pragmatic reasons.
             * Many client implementations send \r\n when
             * the user hits the CarriageReturn key.
             * See RFC 1123 3.3.1 Telnet End-of-Line Convention.
             */
            if (c == '\r' && ptr < end && (*ptr == '\n' || *ptr == '\0'))
                ptr++;
            continue;
        }

        if ((ptr+1) >= end)
            break;
        if (ptr[1] == NOP) { /* Ignore? (putty keepalive, etc.) */
            ptr += 2;
            continue;
        }
        if (ptr[1] == IAC) { /* Literal IAC? (emacs M-DEL) */
            *totty++ = ptr[1];
            ptr += 2;
            continue;
        }

        /*
         * TELOPT_NAWS support!
         */
        if ((ptr+2) >= end) {
            /* Only the beginning of the IAC is in the
            buffer we were asked to process, we can't
            process this char */
            break;
        }
        /*
         * IAC -> SB -> TELOPT_NAWS -> 4-byte -> IAC -> SE
         */
        if (ptr[1] == SB && ptr[2] == TELOPT_NAWS) {
            struct winsize ws;
            if ((ptr+8) >= end)
                break;  /* incomplete, can't process */
            ws.ws_col = (ptr[3] << 8) | ptr[4];
            ws.ws_row = (ptr[5] << 8) | ptr[6];
            ioctl(ts->ptym, TIOCSWINSZ, (char *)&ws);
            ptr += 9;
            continue;
        }
        /* skip 3-byte IAC non-SB cmd */
        ptr += 3;
    }

    num_totty = totty - ptr0;
    *pnum_totty = num_totty;
    /* The difference between ptr and totty is number of iacs
       we removed from the stream. Adjust buf1 accordingly */
    if ((ptr - totty) == 0) /* 99.999% of cases */
        return ptr0;
    ts->wridx1 += ptr - totty;
    ts->size1 -= ptr - totty;
    /* Move chars meant for the terminal towards the end of the buffer */
    return memmove(ptr - num_totty, ptr0, num_totty);
}

VOID  telnet_mode_set(INT32 pty)
{
    struct termios termbuf;

    memset(&termbuf, 0, sizeof(struct termios));
    tcgetattr(pty, &termbuf);
    termbuf.c_lflag &= (~(ICANON | ICANON | ISIG | IEXTEN));
    termbuf.c_lflag &= (~(ECHO | ECHOE | ECHOK | ECHONL));
    termbuf.c_cc[VMIN] = 1;
    termbuf.c_cc[VTIME] = 0;
    tcsetattr(pty, TCSANOW, &termbuf);

    return;
}

INT32  read_one_key(INT32 fd, INT32 *rtn_sym)
{
    INT32  ret = FALSE;
    INT32  kbd_state = KBD_NORMAL;
    INT32  n = 0;
    CHAR  ch = '\0';
    INT32  sym_code = 0;

    if (rtn_sym == NULL)
    {
        ds_assert(0);
        goto  EXIT_LABEL;
    }
    
    do
    {
        n = safe_read(fd, &ch, 1);
        if (n == 0)
        {
            continue;
        }
        else if (n < 0)
        {
            ds_log_err("read form fd %d error\n", fd);
            break;
        }

        switch (kbd_state)
        {
            case KBD_NORMAL:
                switch (ch)
                {
                    case 0x1b:
                        kbd_state = KBD_ESC_A;
                    default:
                        break;
                }
                break;
            case KBD_ESC_A:
                switch (ch)
                {
                    case 0x5b:
                        kbd_state = KBD_ESC_B;
                        break;
                    default:
                        kbd_state = KBD_NORMAL;
                        break;
                }
                break;
            case KBD_ESC_B:
                switch (ch)
                {
                    case 0x41:
                        kbd_state = KBD_NORMAL;
                        sym_code = KEY_ARROW_UP;
                        break;
                    case 0x42:
                        kbd_state = KBD_NORMAL;
                        sym_code = KEY_ARROW_DOWN;
                        break;
                    case 0x43:
                        kbd_state = KBD_NORMAL;
                        sym_code = KEY_ARROW_RIGHT;
                        break;
                    case 0x44:
                        kbd_state = KBD_NORMAL;
                        sym_code = KEY_ARROW_LEFT;
                        break;
                    default:
                        kbd_state = KBD_NORMAL;
                        break;
                }
                break;
            default:
                ds_assert(0);
                break;
        }
    }while (kbd_state != KBD_NORMAL);

    if (sym_code != 0)
    {
        *rtn_sym = sym_code;
    }
    else
    {
        *rtn_sym = ch;
    }
    
    ret = TRUE;
    
EXIT_LABEL:
    return  ret;
}
#endif

#if  DSS_CODE("内部函数")
INT32  dss_telnetd_listen(UINT16 port)
{
    INT32  server_fd = -1;
    struct sockaddr_in srv_addr;
    INT32  ret = -1;

    server_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("DSS: create socket fail!");
        goto EXIT_LABEL;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
        &(int){1}, sizeof(int))==-1)
    {
        perror("DSS: set socket SO_REUSEADDR fail!");
        close(server_fd);
        goto EXIT_LABEL;
    }
    
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(DS_TELNETD_PORT);
    ret = bind(server_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if (ret == -1)
    {
        perror("DSS: bind socket fail!");
        close(server_fd);
        goto EXIT_LABEL;
    }

    ret = listen(server_fd, 1);
    if (ret == -1)
    {
        perror("DSS: listen socket fail!");
        close(server_fd);
        goto EXIT_LABEL;
    }

EXIT_LABEL:
    return  server_fd;
}

VOID* dss_sess_thread_handler(VOID *param)
{
    struct tsession  *ptsess = (struct tsession *)param;
    SESS_MODE cur_sess_mode;
    CHAR cur_sess_mode_str[DS_PROMPT_LEN] = {0};
    CHAR in_buf[DS_MAX_CHARS_PER_LINE] = {0};
    CHAR *trimd_buf = NULL;
    CHAR ch;
    INT32  kbd_sym = 0;
    INT32   collected = 0;
    INT32   offset = 0;
    INT32   newline = 0;
    CHAR  *his_cmd_str = NULL;
    INT32 remaining = 0;
    INT32 i = 0;

    if (ptsess == NULL)
    {
        goto EXIT_LABEL;
    }

    /* print welcome str */
    safe_write(ptsess->ptys, banner_str, strlen(banner_str));
    
    /* print cur prompt str */
    dss_sess_ctx_get_mode_str(ptsess->sess_ctx_env, cur_sess_mode_str);
    safe_write(ptsess->ptys, cur_sess_mode_str, strlen(cur_sess_mode_str));
            
    while (1)
    {
        /*get clean env each time*/
        memset(in_buf, 0, sizeof(in_buf));
        collected = 0;
        offset = 0;
        kbd_sym = 0;
        newline = 0;
        
        /* read one line */
        while ((collected < DS_MAX_CHARS_PER_LINE) && !newline)
        {
            if (!read_one_key(ptsess->ptys, &kbd_sym))
            {
                ds_log_err("read form fd %d error\n", ptsess->ptys);
                goto  EXIT_LABEL;
            }
            
            switch (kbd_sym)
            {
                case KEY_ARROW_UP:
                    dss_sess_ctx_get_mode(ptsess->sess_ctx_env, &cur_sess_mode);
                    if (dss_histroy_prev(ptsess->his_cmd[cur_sess_mode], &his_cmd_str))
                    {
                        for (i = 0; i < collected; ++i)
                        {
                            safe_write(ptsess->ptys, "\010 \010", strlen("\010 \010"));
                        }
                        memcpy(in_buf, his_cmd_str, strlen(his_cmd_str) + 1);
                        
                        collected = strlen(in_buf);
                        offset = collected;
                        
                        //safe_write(ptsess->ptys, KEY_RCP, strlen(KEY_RCP));
                        for (i = 0; i < collected; ++i)
                        {
                            safe_write(ptsess->ptys, &in_buf[i], 1);
                        }
                        safe_write(ptsess->ptys, KEY_CL, strlen(KEY_CL));
                    }
                    else
                    {
                        for (i = 0; i < collected; ++i)
                        {
                            safe_write(ptsess->ptys, "\010 \010", strlen("\010 \010"));
                        }
                        collected = 0;
                        safe_write(ptsess->ptys, KEY_CL, strlen(KEY_CL));
                    }
                    continue;
                case KEY_ARROW_DOWN:
                    dss_sess_ctx_get_mode(ptsess->sess_ctx_env, &cur_sess_mode);
                    if (dss_histroy_next(ptsess->his_cmd[cur_sess_mode], &his_cmd_str))
                    {
                        for (i = 0; i < collected; ++i)
                        {
                            safe_write(ptsess->ptys, "\010 \010", strlen("\010 \010"));
                        }
                        memcpy(in_buf, his_cmd_str, strlen(his_cmd_str) + 1);
                        
                        collected = strlen(in_buf);
                        offset = collected;
                        
                        //safe_write(ptsess->ptys, KEY_RCP, strlen(KEY_RCP));
                        for (i = 0; i < collected; ++i)
                        {
                            safe_write(ptsess->ptys, &in_buf[i], 1);
                        }
                        safe_write(ptsess->ptys, KEY_CL, strlen(KEY_CL));
                    }
                    else
                    {
                        for (i = 0; i < collected; ++i)
                        {
                            safe_write(ptsess->ptys, "\010 \010", strlen("\010 \010"));
                        }
                        collected = 0;
                        safe_write(ptsess->ptys, KEY_CL, strlen(KEY_CL));
                    }
                    continue;
                case KEY_ARROW_RIGHT:
                    if (offset < collected)
                    {
                        safe_write(ptsess->ptys, KEY_RIGHT_STR, strlen(KEY_RIGHT_STR));
                        offset++;
                    }
                    continue;
                case KEY_ARROW_LEFT:
                    if (offset > 0)
                    {
                        safe_write(ptsess->ptys, KEY_LEFT_STR, strlen(KEY_LEFT_STR));
                        offset--;
                    }
                    continue;
                case KEY_CTRL_C:
                    safe_write(ptsess->ptys, "^C\n", strlen("^C\n"));
                    in_buf[0] = '\0';
                    goto EXIT_LABEL;
                case KEY_BACKSPACE:
                    if (collected)
                    {
                        if (!offset)
                        {
                            continue;
                        }
                        safe_write(ptsess->ptys, "\010 \010", strlen("\010 \010"));
                        if (offset != collected)
                        {
                            remaining = collected - offset;
                            for (i = 0; i < remaining; ++i) {
                                safe_write(ptsess->ptys, &in_buf[offset + i], 1);
                                in_buf[offset + i - 1] = in_buf[offset + i];
                            }
                            safe_write(ptsess->ptys, " ", 1);
                            for (i = 0; i < remaining + 1; ++i)
                            {
                                safe_write(ptsess->ptys, KEY_LEFT_STR, strlen(KEY_LEFT_STR));
                            }
                            offset--;
                            collected--;
                        }
                        else
                        {
                            in_buf[--collected] = '\0';
                            offset--;
                        }
                    }
                    continue;
                case '\t':
                    continue;
                case '\n':
                    while (offset < collected)
                    {
                        safe_write(ptsess->ptys, KEY_RIGHT_STR, strlen(KEY_RIGHT_STR));
                        offset++;
                    }
                    if (collected < DS_MAX_CHARS_PER_LINE)
                    {
                        in_buf[collected] = '\n';
                        in_buf[++collected] = '\0';
                        offset++;
                    }
                    safe_write(ptsess->ptys, "\n", 1);
                    newline = 1;
                    continue;
            }

            ch = kbd_sym;
            if (offset != collected)
            {
                for (i = collected; i > offset; --i)
                {
                    in_buf[i] = in_buf[i-1];
                }
                if (collected < DS_MAX_CHARS_PER_LINE)
                {
                    in_buf[offset] = ch;
                    in_buf[++collected] = '\0';
                    offset++;
                }
                for (i = offset - 1; i < collected; ++i)
                {
                    safe_write(ptsess->ptys, &in_buf[i], 1);
                }
                for (i = offset; i < collected; ++i)
                {
                    safe_write(ptsess->ptys, KEY_LEFT_STR, strlen(KEY_LEFT_STR));
                }

            }
            else
            {
                safe_write(ptsess->ptys, &ch, 1);
                if (collected < DS_MAX_CHARS_PER_LINE)
                {
                    in_buf[collected] = ch;
                    in_buf[++collected] = '\0';
                    offset++;
                }
            }
        }

        trimd_buf = ds_comm_trim(in_buf);

        /*Insert to histroy*/
        dss_sess_ctx_get_mode(ptsess->sess_ctx_env, &cur_sess_mode);
        dss_histroy_insert(ptsess->his_cmd[cur_sess_mode], trimd_buf);
        
        if (!dss_cmd_dispatch(ptsess, trimd_buf))
        {
            ds_log_err("Session thread exit:last buf %s\n", trimd_buf);
            break;
        }

        /*After each new line, we print the prompt again*/
        dss_sess_ctx_get_mode_str(ptsess->sess_ctx_env, cur_sess_mode_str);
        safe_write(ptsess->ptys, cur_sess_mode_str, strlen(cur_sess_mode_str));
    }

EXIT_LABEL:
    ptsess->thread_id = 0;
    pthread_exit(NULL);
    return NULL;
}


VOID  dss_sess_free(struct tsession *ts)
{
    struct tsession *tmp_ts = NULL;
    INT32  i = 0;
    
    if (!ts)
    {
        return ;
    }

    if (!(ts->thread_id == 0))
    {
        pthread_cancel(ts->thread_id);
    }
    
    /* Unlink this telnet session from the session list */
    tmp_ts = g_dss_sess_mng.sessions;
    if (tmp_ts == ts)
    {
        g_dss_sess_mng.sessions = ts->next;
    }
    else
    {
        while (tmp_ts->next != ts)
        {
            tmp_ts = tmp_ts->next;
        }
        tmp_ts->next = ts->next;
    }

    close(ts->ptym);
    close(ts->ptys);
    close(ts->sockfd_read);
    /* We do not need to close(ts->sockfd_write), it's the same
     * as sockfd_read unless we are in inetd mode. But in inetd mode
     * we do not reach this */
     
    /* Scan all sessions and find new maxfd */
    g_dss_sess_mng.maxfd = 0;
    tmp_ts = g_dss_sess_mng.sessions;
    while (tmp_ts)
    {
        if (g_dss_sess_mng.maxfd < tmp_ts->ptym)
        {
            g_dss_sess_mng.maxfd = tmp_ts->ptym;
        }
        if (g_dss_sess_mng.maxfd < tmp_ts->sockfd_read)
        {
            g_dss_sess_mng.maxfd = tmp_ts->sockfd_read;
        }

        tmp_ts = tmp_ts->next;
    }
    
    for (i = 0; i < SESS_MODE_MAX; i++)
    {
        if (ts->his_cmd[i] != NULL)
        {
            dss_histroy_uninit(ts->his_cmd[i]);
        }
    }

    free(ts);
    
    if (g_dss_sess_mng.total_sess_num > 0)
    {
        g_dss_sess_mng.total_sess_num--;
    }

}

struct tsession *dss_sess_new(INT32 in_fd)
{
    CHAR pty_name[16] = {0};
    INT32 ret = 0;
    pthread_t sess_tid;
    struct tsession *ts = NULL;
    INT32 i = 0;

    ts = malloc(sizeof(struct tsession) + BUFSIZE * 2);
    if (ts == NULL)
    {
        goto FAIL_LABEL;
    }
    memset(ts, 0, sizeof(struct tsession));
    
    /* Got a new connection, set up a tty. */
    ret = openpty(&ts->ptym, &ts->ptys, pty_name, NULL, NULL);
    if (ret == -1)
    {
        goto FAIL_LABEL;
    }

    if (ts->ptym > g_dss_sess_mng.maxfd)
        g_dss_sess_mng.maxfd = ts->ptym;
    /* Set non blocked */
    DS_FD_SET_NONBLK(ts->ptym);

    ts->sockfd_read = in_fd;
    /* SO_KEEPALIVE by popular demand */
    setsockopt(in_fd, SOL_SOCKET, SO_KEEPALIVE, &(int){1}, sizeof(int));
    DS_FD_SET_NONBLK(in_fd);
    
    ts->sockfd_write = in_fd;
    if (in_fd > g_dss_sess_mng.maxfd)
        g_dss_sess_mng.maxfd = in_fd;
    /* Make the telnet client understand we will echo characters so it
     * should not do it locally. We don't tell the client to run linemode,
     * because we want to handle line editing and tab completion and other
     * stuff that requires CHAR-by-CHAR support. */
    {
        static const CHAR iacs_to_send[] ALIGN1 = {
            IAC, DONT, TELOPT_ECHO,
            IAC, DO, TELOPT_NAWS,
            IAC, DO, TELOPT_LFLOW,
            IAC, WILL, TELOPT_ECHO,
            IAC, WILL, TELOPT_SGA
        };
        memcpy(TS_BUF2, iacs_to_send, sizeof(iacs_to_send));
        ts->rdidx2 = sizeof(iacs_to_send);
        ts->size2 = sizeof(iacs_to_send);
    }

    fflush(NULL); /* flush all streams */
    telnet_mode_set(ts->ptys);

    /* 分配session index */
    g_dss_sess_mng.cur_sess_index++;
    ts->sess_index = g_dss_sess_mng.cur_sess_index;

    dss_sess_ctx_init(&(ts->sess_ctx_env));

    for (i = 0; i < SESS_MODE_MAX; i++)
    {
        ts->his_cmd[i] = dss_histroy_init(10);
        if (ts->his_cmd[i] == NULL)
        {
            goto FAIL_LABEL;
        }
    }

    ret = pthread_create(&sess_tid, NULL, dss_sess_thread_handler, ts);
    if (ret == -1)
    {
        goto FAIL_LABEL;
    }
    ts->thread_id = sess_tid;

    /* detach for resource free */
    pthread_detach(sess_tid);
    
    return ts;

FAIL_LABEL:
    if (ts != NULL)
    {
        if (ts->ptym)
        {
            close(ts->ptym);
        }
        if (ts->ptys)
        {
            close(ts->ptys);
        }
        if (ts->thread_id)
        {
            pthread_cancel(ts->thread_id);
        }

        dss_sess_ctx_uninit(&(ts->sess_ctx_env));
        
        for (i = 0; i < SESS_MODE_MAX; i++)
        {
            if (ts->his_cmd[i] != NULL)
            {
                dss_histroy_uninit(ts->his_cmd[i]);
            }
        }
        
        free(ts);
    }
    return  NULL;
}
#endif

inline DSS_SESS_CTX*  dss_sess_get_ctx(struct tsession  *ts)
{
    ds_assert(ts);

    return  ts->sess_ctx_env;
}
inline INT32  dss_sess_get_idx(struct tsession  *ts)
{
    ds_assert(ts);

    return  ts->sess_index;
}

VOID  dss_sess_write_term(INT32 sess_index, UINT8 *msg, UINT32 msg_len)
{
    struct tsession *ts = NULL;
    struct tsession *next = NULL;
    INT32  count = 0;
    
    ts = g_dss_sess_mng.sessions;
    while (ts) {
        next = ts->next;

        if (ts->sess_index == sess_index)
        {
            count = safe_write(ts->ptys, msg, msg_len);
            if (count < 0)
            {
                if (errno == EAGAIN)
                {
                }
                else
                {
                    dss_sess_free(ts);
                }
            }
            break;
        }
        ts = next;
    }
    
    return ;
}


VOID  dss_sess_write_all_term(UINT8 *msg, UINT32 msg_len)
{
    struct tsession *ts = NULL;
    struct tsession *next = NULL;
    INT32  count = 0;
    
    ts = g_dss_sess_mng.sessions;
	while (ts) {
	    next = ts->next;
        count = safe_write(ts->ptys, msg, msg_len);
        if (count < 0)
        {
            if (errno == EAGAIN)
            {
                ts = next;
                continue;
            }
            else
            {
                dss_sess_free(ts);
            }
        }
	    ts = next;
	}
	
    return ;
}


INT32  dss_telnetd_start(VOID)
{
    INT32  listen_fd = -1;
    fd_set rdfdset, wrfdset;
    INT32 count = 0;
    struct tsession *ts = NULL;

    /*创建telnet服务监听fd*/
    listen_fd = dss_telnetd_listen(DS_TELNETD_PORT);
    while (listen_fd == -1)
    {
        sleep(5);
        listen_fd = dss_telnetd_listen(DS_TELNETD_PORT);
    }
    
    /*
       This is how the buffers are used. The arrows indicate data flow.
    
       +-------+     wridx1++     +------+     rdidx1++     +----------+
       |       | <--------------  | buf1 | <--------------  |          |
       |       |     size1--      +------+     size1++      |          |
       |  pty  |                                            |  socket  |
       |       |     rdidx2++     +------+     wridx2++     |          |
       |       |  --------------> | buf2 |  --------------> |          |
       +-------+     size2++      +------+     size2--      +----------+
    
       size1: "how many bytes are buffered for pty between rdidx1 and wridx1?"
       size2: "how many bytes are buffered for socket between rdidx2 and wridx2?"
    
       Each session has got two buffers. Buffers are circular. If sizeN == 0,
       buffer is empty. If sizeN == BUFSIZE, buffer is full. In both these cases
       rdidxN == wridxN.
    */

 again:
    FD_ZERO(&rdfdset);
    FD_ZERO(&wrfdset);

    /* Select on the master socket, all telnet sockets and their
     * ptys if there is room in their session buffers.
     * NB: scalability problem: we recalculate entire bitmap
     * before each select. Can be a problem with 500+ connections. */
    ts = g_dss_sess_mng.sessions;
	while (ts) {
		struct tsession *next = ts->next; /* in case we free ts */
		if (ts->thread_id == 0) {
			/* Child died and we detected that */
			dss_sess_free(ts);
		} else {
			if (ts->size1 > 0)       /* can write to pty */
				FD_SET(ts->ptym, &wrfdset);
			if (ts->size1 < BUFSIZE) /* can read from socket */
				FD_SET(ts->sockfd_read, &rdfdset);
			if (ts->size2 > 0)       /* can write to socket */
				FD_SET(ts->sockfd_write, &wrfdset);
			if (ts->size2 < BUFSIZE) /* can read from pty */
				FD_SET(ts->ptym, &rdfdset);
		}
		ts = next;
	}
	
    /* Add listen fd to read set, then update the max fd */
    FD_SET(listen_fd, &rdfdset);
    if (listen_fd > g_dss_sess_mng.maxfd)
    {
        g_dss_sess_mng.maxfd = listen_fd;
    }

    count = select(g_dss_sess_mng.maxfd + 1, &rdfdset, &wrfdset, NULL, NULL);
    if (count < 0)
    {
        goto again; /* EINTR or ENOMEM */
    }
        
    /* check is there any accept in connection, if any then make new sess */
    if (FD_ISSET(listen_fd, &rdfdset)) {
        INT32 fd;
        struct tsession *new_ts = NULL;

        fd = accept(listen_fd, NULL, NULL);
        if (fd < 0)
        {
            goto again;
        }
                    
        /* Create a new session and link it into our active list */
        if (g_dss_sess_mng.total_sess_num < DS_TELNETD_MAX_SESS_NUM)
        {
            new_ts = dss_sess_new(fd);
            g_dss_sess_mng.total_sess_num++;
            
            if (new_ts) {
			new_ts->next = g_dss_sess_mng.sessions;
			g_dss_sess_mng.sessions = new_ts;
    		} else {
    			close(fd);
    		}
        }
        else
        {
            close(fd);
        }
    }

    /* Then check for data tunneling. */
    
    ts = g_dss_sess_mng.sessions;
	while (ts) {
		struct tsession *next = ts->next; /* in case we free ts */
        if (ts->size1 && FD_ISSET(ts->ptym, &wrfdset)) {
            INT32 num_totty;
            UINT8 *ptr;
            /* Write to pty from buffer 1. */
            ptr = remove_iacs(ts, &num_totty);
            count = safe_write(ts->ptym, ptr, num_totty);
            if (count < 0) {
                if (errno == EAGAIN)
                    goto skip1;
                goto kill_session;
            }

            ts->size1 -= count;
            ts->wridx1 += count;
            if (ts->wridx1 >= BUFSIZE) /* actually == BUFSIZE */
                ts->wridx1 = 0;
        }
 skip1:
        if (ts->size2 && FD_ISSET(ts->sockfd_write, &wrfdset)) {
            /* Write to socket from buffer 2. */
            count = MIN(BUFSIZE - ts->wridx2, ts->size2);
            count = safe_write(ts->sockfd_write, TS_BUF2 + ts->wridx2, count);
            if (count < 0) {
                if (errno == EAGAIN)
                    goto skip2;
                goto kill_session;
            }
            ts->size2 -= count;
            ts->wridx2 += count;
            if (ts->wridx2 >= BUFSIZE) /* actually == BUFSIZE */
                ts->wridx2 = 0;
        }
 skip2:
        /* Should not be needed, but... remove_iacs is actually buggy
         * (it cannot process iacs which wrap around buffer's end)!
         * Since properly fixing it requires writing bigger code,
         * we rely instead on this code making it virtually impossible
         * to have wrapped iac (people don't type at 2k/second).
         * It also allows for bigger reads in common case. */
        if (ts->size1 == 0) {
            ts->rdidx1 = 0;
            ts->wridx1 = 0;
        }
        if (ts->size2 == 0) {
            ts->rdidx2 = 0;
            ts->wridx2 = 0;
        }

        if (ts->size1 < BUFSIZE && FD_ISSET(ts->sockfd_read, &rdfdset)) {
            /* Read from socket to buffer 1. */
            count = MIN(BUFSIZE - ts->rdidx1, BUFSIZE - ts->size1);
            count = safe_read(ts->sockfd_read, TS_BUF1 + ts->rdidx1, count);
            if (count <= 0) {
                if (count < 0 && errno == EAGAIN)
                    goto skip3;
                goto kill_session;
            }
            /* Ignore trailing NUL if it is there */
            if (!TS_BUF1[ts->rdidx1 + count - 1]) {
                --count;
            }
            ts->size1 += count;
            ts->rdidx1 += count;
            if (ts->rdidx1 >= BUFSIZE) /* actually == BUFSIZE */
                ts->rdidx1 = 0;
        }
 skip3:
        if (ts->size2 < BUFSIZE && FD_ISSET(ts->ptym, &rdfdset)) {
            /* Read from pty to buffer 2. */
            count = MIN(BUFSIZE - ts->rdidx2, BUFSIZE - ts->size2);
            count = safe_read(ts->ptym, TS_BUF2 + ts->rdidx2, count);
            if (count <= 0) {
                if (count < 0 && errno == EAGAIN)
                    goto skip4;
                goto kill_session;
            }
            ts->size2 += count;
            ts->rdidx2 += count;
            if (ts->rdidx2 >= BUFSIZE) /* actually == BUFSIZE */
                ts->rdidx2 = 0;
        }
 skip4:
        ts = next;
        continue;

 kill_session:
        dss_sess_free(ts);
        ts = next;
    }

    goto again;
}


