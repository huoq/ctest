/*****************************************************************************/
/** 
* \file       ds_symbol_mgr.h
* \author     hejian
* \date       2015/06/09
* \version    FlexBNG V1
* \brief      符号管理头文件
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#ifndef _DS_PARSE_SYMBOL_H_
#define _DS_PARSE_SYMBOL_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* symbol types */
#define SYM_UNDF        0x0     /* undefined */
#define SYM_LOCAL       0x0     /* local */
#define SYM_GLOBAL      0x1     /* global (external) (ORed) */
#define SYM_ABS         0x2     /* absolute */
#define SYM_TEXT        0x4     /* text */
#define SYM_DATA        0x6     /* data */
#define SYM_BSS         0x8     /* bss */
#define SYM_COMM        0x12    /* common symbol */
#define SYM_SDA         0x40    /* symbols related to a PowerPC SDA section */
#define SYM_SDA2        0x80    /* symbols related to a PowerPC SDA2 section */

#define SYM_THUMB   0x40    /* Thumb function */

/* symbol masks */
#define SYM_MASK_ALL    0xff            /* all bits of symbol type valid */
#define SYM_MASK_NONE   0x00        /* no bits of symbol type valid */
#define SYM_MASK_EXACT  0x1ff       /* match symbol pointer exactly */
#define SYM_SDA_MASK    0xc0            /* for SDA and SDA2 symbols */

#define SYM_MASK_ANY_TYPE    SYM_MASK_NONE  /* ignore type in searches */
#define SYM_MASK_EXACT_TYPE  SYM_MASK_ALL   /* match type exactly in searches */

/*
 * Simple values for n_type.
 */

#define N_UNDF  0x0     /* undefined */
#define N_ABS   0x2     /* absolute */
#define N_TEXT  0x4     /* text */
#define N_DATA  0x6     /* data */
#define N_BSS   0x8     /* bss */
#define N_COMM  0x12        /* common (internal to ld) */
#define N_FN    0x1f        /* file name symbol */

#define N_EXT   01      /* external bit, or'ed in */
#define N_TYPE  0x1e        /* mask for all the type bits */

/*
 * Sdb entries have some of the N_STAB bits set.
 * These are given in <stab.h>
 */

#define N_STAB  0xe0        /* if any of these bits set, a SDB entry */

#define SYM_MASK_NONE   0x00    /* symbol masks, no bits of symbol type valid */
#define MAX_SYS_SYM_LEN 128

typedef UINT8 SYM_TYPE;     /* SYM_TYPE */
typedef struct  tagSYMBOL   /* SYMBOL - entry in symbol table */
{
    TAILQ_ENTRY(tagSYMBOL) symNode;
    CHAR    *name;        /* pointer to symbol name */
    VOID    *value;     /* symbol value */
    UINT16  group;        /* symbol group */
    SYM_TYPE    type;        /* symbol type */
} SYMBOL;

typedef SYMBOL * SYMBOL_ID;


typedef struct symtab   /* SYMTAB - symbol table */
{
    TAILQ_HEAD(symHead, tagSYMBOL) symList;
    INT32     nsymbols;    /* current number of symbols in table */
} SYMTAB;

typedef SYMTAB *SYMTAB_ID;


STATUS symAdd
(
    CHAR      *name,            /* pointer to symbol name string */
    CHAR      *value,            /* symbol address */
    SYM_TYPE  type             /* symbol type */
);
STATUS symFindByName
(
    CHAR        *name,          /* symbol name to look for */
    CHAR        **pValue,       /* where to put symbol value */
    SYM_TYPE    *pType          /* where to put symbol type */
);
STATUS symFindSymbol
(
    CHAR *      name,           /* name to search for */
    VOID *  value,        /* value of symbol to search for */
    SYMBOL_ID * pSymbolId       /* where to return pointer to matching symbol */
);

STATUS symNameGet
(
    SYMBOL_ID  symbolId,
    CHAR **    pName
);

STATUS symValueGet 
(
    SYMBOL_ID  symbolId,
    VOID **    pValue
);

STATUS symTypeGet 
(
    SYMBOL_ID  symbolId,
    SYM_TYPE * pType
);

INT32  ds_symbol_mgr_init(VOID);
VOID   ds_symbol_mgr_uninit(VOID);

#ifdef __cplusplus
}
#endif
#endif
