/*****************************************************************************/
/** 
* \file       ds_symbol_mgr.c
* \author     hejian
* \date       2015/06/09
* \version    FlexBNG V1
* \brief      符号管理实现
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "ds_comm_incl.h"

SYMTAB_ID sysSymTbl;
UINT16 symGroupDefault = 0;


LOCAL STATUS symInit
(
SYMBOL      *pSymbol,       /* pointer to symbol */
CHAR        *name,          /* pointer to symbol name string */
CHAR        *value,         /* symbol address */
SYM_TYPE    type,           /* symbol type */
UINT16  group       /* symbol group */
)
{
    /* fill in symbol */

    pSymbol->name  = name;            /* symbol name */
    pSymbol->value = (VOID *)value;        /* symbol value */
    pSymbol->type  = type;            /* symbol type */
    pSymbol->group = group;            /* symbol group */

    return (OK);
}


SYMBOL *symAlloc
    (
    SYMTAB_ID   symTblId,       /* symbol table to allocate symbol for */
    CHAR        *name,          /* pointer to symbol name string */
    CHAR        *value,         /* symbol address */
    SYM_TYPE    type,           /* symbol type */
    UINT16  group       /* symbol group */
    )
{
    SYMBOL *pSymbol;
    CHAR   *symName;
    INT32    length;

    if (name == NULL)
    return (NULL);                /* null name */

    length = strlen (name);            /* figure out name length */

    pSymbol = (SYMBOL *) malloc((UINT32)(sizeof(SYMBOL) + length + 1));

    if (pSymbol == NULL)            /* out of memory */
    return (NULL);

    /* copy name after symbol */

    symName     = (CHAR *) ((UINT8*) pSymbol + sizeof (SYMBOL));
    symName[length] = EOS;            /* null terminate string */

    strncpy (symName, name, length);        /* copy name into place */

    symInit (pSymbol, symName, value, type, group); /* initialize symbol*/

    return (pSymbol);                /* return symbol ID */
}

STATUS symFree
    (
    SYMBOL *pSymbol     /* pointer to symbol to delete */
    )
{
    if (pSymbol == NULL)
    {
        return ERROR;
    }

    free(pSymbol);

    return OK;
}


STATUS symTblAdd
(
    SYMTAB_ID symTblId,         /* symbol table to add symbol to */
    SYMBOL    *pSymbol          /* pointer to symbol to add */
)
{
    TAILQ_INSERT_HEAD(&(symTblId->symList), pSymbol, symNode);

    symTblId->nsymbols++;            /* increment symbol count */

    return (OK);
}

STATUS symAdd
(
CHAR      *name,            /* pointer to symbol name string */
CHAR      *value,           /* symbol address */
SYM_TYPE  type             /* symbol type */
)
{
    SYMBOL *pSymbol = symAlloc (sysSymTbl, name, value, type, symGroupDefault);

    if (pSymbol == NULL)            /* out of memory? */
    return (ERROR);

    if (symTblAdd (sysSymTbl, pSymbol) != OK)    /* try to add symbol */
    {
        free(pSymbol);        /* deallocate symbol if fail */
        return (ERROR);
    }

    return (OK);
}

STATUS symFindByName
(
    CHAR        *name,          /* symbol name to look for */
    CHAR        **pValue,       /* where to put symbol value */
    SYM_TYPE    *pType          /* where to put symbol type */
)
{
    SYMBOL_ID  pSymId = NULL;
    VOID *funptr = NULL;

    /*First find use dl lib*/

    funptr = dlsym(NULL, name);
    if (funptr != NULL)
    {
        if (pValue != NULL)
        {
            *pValue = (CHAR *) funptr;
        }
        
        if (pType != NULL)
        {
            *pType = (N_BSS | N_EXT);
        }

        return  (OK);
    }

    
    if (sysSymTbl == NULL)
    {
        goto FAIL_LABEL;
    }

    TAILQ_FOREACH(pSymId, &(sysSymTbl->symList), symNode)
    {
        if (!strcmp(pSymId->name, name))
        {
            break;
        }
    }
    
    if (pSymId == NULL)
    {
        goto FAIL_LABEL;
    }
    
    if (pValue != NULL)
    {
        *pValue = (CHAR *) pSymId->value;
    }
    
    if (pType != NULL)
    {
        *pType = pSymId->type;
    }
        
    return  (OK);

FAIL_LABEL:
    return  (ERROR);
}

STATUS symFindSymbol
(
    CHAR *      name,           /* name to search for */
    VOID *  value,        /* value of symbol to search for */
    SYMBOL_ID * pSymbolId       /* where to return pointer to matching symbol */
)
{
    SYMBOL_ID  pSymId = NULL;

    if (sysSymTbl == NULL)
    {
        goto FAIL_LABEL;
    }
    
    TAILQ_FOREACH(pSymId, &(sysSymTbl->symList), symNode)
    {
        if (name != NULL)
        {
            if (!strcmp(pSymId->name, name))
            {
                if (pSymId->value == value)
                {
                    break;
                }
            }
        }
        else
        {
            if (pSymId->value == value)
            {
                break;
            }
        }
    }

    if (pSymId == NULL)
    {
        goto FAIL_LABEL;
    }
    
     *pSymbolId = pSymId;
    return  (OK);

FAIL_LABEL:
    return  (ERROR);
}

STATUS symNameGet
(
    SYMBOL_ID  symbolId,
    CHAR **    pName
)
{
    if ((symbolId == NULL) || (pName == NULL))
    return ERROR;

    *pName = symbolId->name;

    return OK;
}

STATUS symValueGet 
(
    SYMBOL_ID  symbolId,
    VOID **    pValue
)
{
    if ((symbolId == NULL) || (pValue == NULL))
    return ERROR;

    *pValue = symbolId->value;

    return OK;
}

STATUS symTypeGet 
(
    SYMBOL_ID  symbolId,
    SYM_TYPE * pType
)
{
    if ((symbolId == NULL) || (pType == NULL))
    return ERROR;

    *pType = symbolId->type;

    return OK;
}

INT32  ds_symbol_mgr_init(VOID)
{
    sysSymTbl = malloc(sizeof(SYMTAB));
    if (sysSymTbl == NULL)
    {
        goto  FAIL_LABEL;
    }
    memset(sysSymTbl, 0, sizeof(SYMTAB));

    sysSymTbl->nsymbols = 0;
    TAILQ_INIT(&sysSymTbl->symList);
    
    return  TRUE;
FAIL_LABEL:
    return  FALSE;
}

VOID  ds_symbol_mgr_uninit(VOID)
{
    SYMBOL_ID  pSymId = NULL;
    SYMBOL_ID  pTmpSymId = NULL;
    
    TAILQ_FOREACH_SAFE(pSymId, &(sysSymTbl->symList), symNode, pTmpSymId)
    {
        TAILQ_REMOVE(&(sysSymTbl->symList), pSymId, symNode);
        symFree(pSymId);
    }
    
    free(sysSymTbl);
    return;
}

INT32  ds_symbol_add(CHAR *name, CHAR *value, INT32 type)
{
    return symAdd(name, value, type);
}
