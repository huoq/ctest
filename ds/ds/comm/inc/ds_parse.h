/*****************************************************************************/
/** 
* \file       ds_parse.h
* \author     hejian
* \date       2015/06/04
* \version    FlexBNG V1
* \brief      解析器头文件
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#ifndef _DS_PARSE_H_
#define _DS_PARSE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define  YYSTYPE VALUE       /* type of parse stack */
#define  MAX_SHELL_LINE  256 /* max chars on line typed to shell */
#define  MAX_SHELL_ARGS 30   /* max number of args on stack */
#define  MAX_FUNC_ARGS  12   /* max number of args to any one function */

#define BIN_OP(op)  rvOp((getRv(&yypvt[-2], &tmpVal1)), op, \
                  getRv(&yypvt[-0], &tmpVal2))

#define RV(value)    (getRv (&(value), &tmpVal2))
#define NULLVAL     (VALUE *) NULL

#define CHECK       if (semError) YYERROR
#define SET_ERROR   semError = TRUE

#define EOS '\0'

#define NL 0
#define T_SYMBOL 258
#define D_SYMBOL 259
#define U_SYMBOL 260
#define NUMBER 261
#define DS_CHAR 262
#define STRING 263
#define DS_FLOAT 264
#define OR 265
#define AND 266
#define EQ 267
#define NE 268
#define GE 269
#define LE 270
#define INCR 271
#define DECR 272
#define ROT_LEFT 273
#define ROT_RIGHT 274
#define UMINUS 275
#define PTR 276
#define TYPECAST 277
#define ENDFILE 278
#define LEX_ERROR 279
#define MULA 280
#define DIVA 281
#define MODA 282
#define ADDA 283
#define SUBA 284
#define SHLA 285
#define SHRA 286
#define ANDA 287
#define ORA 288
#define XORA 289
#define UNARY 290

#define NELEMENTS(x) ((INT32)(sizeof(x)/sizeof(x[0])))

typedef  ULONG (*FUNCPTR)(ULONG, ...);
#define IMPORT  extern
#define LOCAL   static
#define FAST    register

typedef enum
{
    OK,
    ERROR=-1
}STATUS;

/********************************************************************************
 *                               struct                                         *
 ********************************************************************************/
typedef enum        /* TYPE */
    {
    T_UNKNOWN,
    T_BYTE,
    T_WORD,
#ifndef _WRS_NO_TGT_SHELL_FP
    T_INT,
    T_FLOAT,
    T_DOUBLE
#else   /* _WRS_NO_TGT_SHELL_FP */
    T_INT
#endif  /* _WRS_NO_TGT_SHELL_FP */
    } TYPE;

typedef enum        /* SIDE */
    {
    LHS,
    RHS,
    FHS         /* function: rhs -> lhs */
    } SIDE;

typedef struct      /* VALUE */
    {
    SIDE side;
    TYPE type;
    union
    {
    int64_t *lv;    /* pointer to any of the below */

    char byte;
    short word;
    ULONG rv;
    char *string;
#ifndef _WRS_NO_TGT_SHELL_FP
    float fp;
    double dp;
#endif  /* _WRS_NO_TGT_SHELL_FP */
    } value;
    } VALUE;

#ifdef __cplusplus
}
#endif
#endif
