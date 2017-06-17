/*****************************************************************************/
/** 
* \file       dsa_io.h
* \author     hejian
* \date       2015/06/08
* \version    FlexBNG V1
* \brief      DSA输出重定向处理头文件
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/

#ifndef  _DSA_IO_H_
#define  _DSA_IO_H_

#ifdef  __cplusplus
extern "C"
{
#endif

INT32  dsa_io_init(DS_EVENT_MNG* evt_mng);
VOID   dsa_io_uninit(VOID);


#ifdef  __cplusplus
}
#endif
#endif
