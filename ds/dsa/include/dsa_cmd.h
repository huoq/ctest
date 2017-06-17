/*****************************************************************************/
/** 
* \file       dsa_cmd.h
* \author     hejian
* \date       2015/06/08
* \version    FlexBNG V1
* \brief      处理从用户端过来的命令
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/

#ifndef  _DSA_CMD_H_
#define  _DSA_CMD_H_

#ifdef  __cplusplus
extern "C"
{
#endif

INT32  dsa_cmd_init(VOID);
INT32  dsa_cmd_dispatch(CHAR *cmd_str);

#ifdef  __cplusplus
}
#endif
#endif
