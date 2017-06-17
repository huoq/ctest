/*****************************************************************************/
/** 
* \file       dss_sess_histroy.h
* \author     hejian
* \date       2015/06/04
* \version    FlexBNG V1
* \brief      �ն��ַ���������
* \note       Copyright (c) 2000-2020  ����˹��Ϣ�Ƽ��ɷ����޹�˾
* \remarks    �޸���־
******************************************************************************/
#ifndef  _DSS_TERM_KEY_H_
#define  _DSS_TERM_KEY_H_

#ifdef  __cplusplus
extern "C"
{
#endif

/*�ն˵��ַ��Ͱ�����Ӧ��ϵ����, from klange project*/

#define KEY_NONE        0
#define KEY_BACKSPACE   8
#define KEY_CTRL_C      3
#define KEY_CTRL_L      12
#define KEY_CTRL_R      18
#define KEY_ESCAPE      27
#define KEY_NORMAL_MAX  256
#define KEY_ARROW_UP    257
#define KEY_ARROW_DOWN  258
#define KEY_ARROW_RIGHT 259
#define KEY_ARROW_LEFT  260

#define KEY_UP_STR     "\033[A"  /*up arrow*/
#define KEY_DOWN_STR   "\033[B"  /*down arrow*/
#define KEY_LEFT_STR   "\033[D"  /*left arrow*/
#define KEY_RIGHT_STR  "\033[C"  /*right arrow*/
#define KEY_RCP        "\033[u"  /* Restore cursor position */
#define KEY_CL         "\033[K"  /* clear line from actual position to end of the line */

/*����״̬*/
#define KBD_NORMAL 0
#define KBD_ESC_A  1
#define KBD_ESC_B  2

#define KBD_NULL  -1
#define KBD_CHAR  0
#define KBD_CTL   1

#ifdef  __cplusplus
}
#endif

#endif
