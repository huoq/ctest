/*****************************************************************************/
/** 
* \file       ds_parse_lex.c
* \author     hejian
* \date       2015/06/04
* \version    FlexBNG V1
* \brief      词法分析器实现
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "ds_comm_incl.h"

IMPORT BOOL sysAdaEnable;    /* TRUE = enable Ada support */
IMPORT BOOL sysCplusEnable;    /* TRUE = enable C++ support */
typedef enum        /* states for automatic insertion of parens in yylex */
{
    FIRST_LEXEME,
    NORMAL,
    P_OPEN,
    P_OPEN_DONE,
    P_CLOSE,
    P_CLOSE_DONE
} AUTO_STATE;
LOCAL AUTO_STATE autoState;        /* state of auto parens mods */
LOCAL char *nextChar;            /* ptr to next input char in line */
LOCAL char tempStrings [MAX_SHELL_LINE];/* storage for strings while parsing */
LOCAL char *nextTempString;        /* ptr to free space in tempStrings */

extern YYSTYPE ds_yylval;

/* forward declaration */
static char *addTempString (char *string);
static void lexError (char *string, char *errmsg);
static int64_t getNum (char *string, char *fmtString, VALUE *pValue);
static int getFloat (char *string, VALUE *pValue);
static int getString (char *string, int nChars, VALUE *pValue);
static int getChar (char *string, int nChars, VALUE *pValue);
static int getId (char *string, VALUE *pValue);
static int typeCast (char *string);
static int strToChar (char *string, char *pChar);
static void lexInit (void);
static int64_t lexScan (void);
static void lexRetract (void);
static int lexNclasses;
#define RETRACT     lexRetract (); string[--nChars] = EOS
static int64_t lexActions (state, string, nChars, pContinue)
    int state; char *string; int nChars; BOOL*pContinue;
    {
    *pContinue = FALSE;
    switch (state)
        {
        case 1:
            { RETRACT;    } break;
        case 2:
            { RETRACT;  return (string[0]); } break;
        case 3:
            {       return (string[0]); } break;
        case 4:
            {       lexError(string, "invalid number"); return(LEX_ERROR); } break;
        case 5:
            {       lexError(string, "invalid string"); return(LEX_ERROR); } break;
        case 6:
            {       lexError(string, "invalid char");   return(LEX_ERROR); } break;
        case 7:
            { RETRACT;  return (getNum (string, "%o", &ds_yylval)); } break;
        case 8:
            { RETRACT;  return (getNum (&string[2], "%" SCNx64, &ds_yylval)); } break;
        case 9:
            { RETRACT;  return (getNum (&string[1], "%" SCNx64, &ds_yylval)); } break;
        case 10:
            { RETRACT;  return (getNum (string, "%d", &ds_yylval)); } break;
        case 11:
            { RETRACT;  return (getFloat (string, &ds_yylval)); } break;
        case 12:
            { RETRACT;  return (getId (string, &ds_yylval)); } break;
        case 13:
            {       return (getString (string, nChars, &ds_yylval)); } break;
        case 14:
            {       return (getChar (string, nChars, &ds_yylval)); } break;
        case 15:
            {       return (OR); } break;
        case 16:
            {       return (AND); } break;
        case 17:
            {       return (EQ); } break;
        case 18:
            {       return (NE); } break;
        case 19:
            {       return (GE); } break;
        case 20:
            {       return (LE); } break;
        case 21:
            { RETRACT;  return (ROT_RIGHT); } break;
        case 22:
            { RETRACT;  return (ROT_LEFT); } break;
        case 23:
            {       return (PTR); } break;
        case 24:
            {       return (INCR); } break;
        case 25:
            {       return (DECR); } break;
        case 26:
            {       return (ADDA); } break;
        case 27:
            {       return (SUBA); } break;
        case 28:
            {       return (MULA); } break;
        case 29:
            {       return (DIVA); } break;
        case 30:
            {       return (MODA); } break;
        case 31:
            {       return (SHLA); } break;
        case 32:
            {       return (SHRA); } break;
        case 33:
            {       return (ANDA); } break;
        case 34:
            {       return (ORA); } break;
        case 35:
            {       return (XORA); } break;
        case 36:
            {       return (NL); } break;
        case 37:
            {       return (ENDFILE); } break;
        }
    *pContinue = TRUE;
    return (0);
    }

static int lexNclasses = 27;

static char lexClass [] =
    {
    26,
    25,  0,  0,  0, 26,  0,  0,  0,  0,  1, 25,  0,  0, 25,  0,  0, 
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
     1, 16, 10,  0, 12, 23, 14, 11,  0,  0, 22, 20,  0, 19,  8, 21, 
     2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  0,  0, 18, 15, 17,  0, 
     0,  5,  5,  5,  5,  5,  5,  7,  7,  7,  7,  7,  7,  7,  7,  7, 
     7,  7,  7,  7,  7,  7,  7,  7,  6,  7,  7,  0,  9,  0, 24,  7, 
     0,  5,  5,  5,  5,  5,  5,  7,  7,  7,  7,  7,  7,  7,  7,  7, 
     7,  7,  7,  7,  7,  7,  7,  7,  6,  7,  7,  0, 13,  0,  0,  0, 
    };

static char lexStateTable [] =
    {
    -3, 1, 2, 8, 8,11,11,11, 9,-3,12,14, 6,19,20,21,22,23,24,25,26,16,27,28,29,-36,-37,
    -1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -10,-10, 3, 3,-4,-4, 4,-4,10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,
    -7,-7, 3, 3,-4,-4,-4,-4,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,
    -4,-4, 5, 5, 5, 5,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,
    -8,-8, 5, 5, 5, 5,-4,-4,-4,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,
    -4,-4, 7, 7, 7, 7,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,
    -9,-9, 7, 7, 7, 7,-4,-4,-4,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,
    -10,-10, 8, 8, 8,-4,-4,-4,10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,
    -11,-11,10,10,10,-4,-4,-4,-4,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,
    -11,-11,10,10,10,-4,-4,-4,-4,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,
    -12,-12,11,11,11,11,11,11,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,
    12,12,12,12,12,12,12,12,12,13,-13,12,12,12,12,12,12,12,12,12,12,12,12,12,12,-5,-5,
    12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,-5,-5,
    14,14,14,14,14,14,14,14,14,15,14,-14,14,14,14,14,14,14,14,14,14,14,14,14,14,-6,-6,
    14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,-6,-6,
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-29,-2,-2,-2,-2,-2,-2,17,-2,-2,-2,-2,
    17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,18,17,17,17,17,
    17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17, 0,17,17,17,17,17,
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-15,-2,-34,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-16,-33,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-17,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-18,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-19,-2,30,-2,-2,-2,-2,-2,-2,-2,-2,-2,
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-20,-2,-2,31,-2,-2,-2,-2,-2,-2,-2,-2,
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-27,-2,-23,-2,-25,-2,-2,-2,-2,-2,-2,-2,
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-26,-2,-2,-2,-2,-24,-2,-2,-2,-2,-2,-2,
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-28,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-30,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
    -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-35,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
    -21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-32,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,
    -22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-31,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,
    };
/*******************************************************************************
*
* ds_lexNewLine - initialize for lexical scan of new line
*
* RETURNS: N/A
*/
void ds_lexNewLine(char *line)
{
    lexInit ();
    nextChar = line;
    nextTempString = tempStrings;
    autoState = FIRST_LEXEME;
}
/*******************************************************************************
*
* ds_yylex - get next lexeme for yacc
*
* This routine is called by yacc to get the next input lexeme.
* In addition to simply calling lexScan to do the scan, this routine
* also handles the automatic insertion of parens around the arguements
* of a "top-level" routine call.  If the first lexeme scanned from a new
* line is a T_SYMBOL (text id) and the second lexeme is NOT a '(',
* then the second lexeme is withheld and a '(' returned instead.
* The withheld lexeme is returned next.  Scanning then proceeds normally
* until a NL (newline) lexeme is scanned.  The NL is withheld and a
* ')' is returned instead, with the NL being returned next.
*
* RETURNS: next lexeme.
*/
int64_t ds_yylex (void)
    {
    static int64_t heldCode;
    FAST int64_t code;
    switch (autoState)
    {
        case FIRST_LEXEME:        /* first lex scan of new line */
        code = lexScan ();
        autoState = (code == T_SYMBOL) ? P_OPEN : NORMAL;
        break;
    case NORMAL:            /* parens not required to be inserted */
        code = lexScan ();
        if (code == ';')
        autoState = FIRST_LEXEME;
        break;
    case P_OPEN:            /* looking for '(' */
        code = lexScan ();
        if (code == '(')
        autoState = NORMAL;
        else
        {
        heldCode = code;
        code = '(';
        autoState = P_OPEN_DONE;
        }
        break;
    case P_OPEN_DONE:        /* artificial '(' has been returned */
        if ((heldCode == NL) || (heldCode == ';'))
        {
        code = ')';
        autoState = P_CLOSE_DONE;
        }
        else
        {
        code = heldCode;
        autoState = P_CLOSE;
        }
        break;
    case P_CLOSE:            /* looking for NL or ';' */
        code = lexScan ();
        if ((code == NL) || (code == ';'))
        {
        heldCode = code;
        code = ')';
        autoState = P_CLOSE_DONE;
        }
        break;
    case P_CLOSE_DONE:        /* artificial ')' has been returned */
        code = heldCode;
        autoState = FIRST_LEXEME;
        break;
    default:
        DS_PRINTF ("ds_yylex: invalid state %#x\n", autoState);
        code = 0;    /* invalid? */
        break;
    }
    return (code);
    }
/*******************************************************************************
*
* addTempString - add string to temporary storage
*
* This routine adds the specified string to the during-parse temporary
* string storage.
*
* RETURNS: pointer to new string appended to temporary area.
*/
LOCAL char *addTempString
    (
    char *string
    )
    {
    char *newString = nextTempString;
    while (*string != EOS)
    string += strToChar (string, nextTempString++);
    *(nextTempString++) = EOS;
    return (newString);
    }
/*******************************************************************************
*
* lexError - report error in lex scan
*
* RETURNS: N/A
*/
LOCAL void lexError
    (
    char *string,
    char *errmsg
    )
    {
    DS_PRINTF ("%s: %s\n", errmsg, string);
    }
/*******************************************************************************
*
* getNum - interpret scanned string as integer
*
* RETURNS: NUMBER
*/
LOCAL int64_t getNum
    (
    char *string,
    char *fmtString,
    VALUE *pValue
    )
    {
    pValue->side = RHS;
    pValue->type = T_INT;
    sscanf (string, fmtString, &pValue->value.rv);
    return (NUMBER);
    }
/*******************************************************************************
*
* getFloat - interpret scanned string as float
*
* RETURNS: FLOAT
*/
LOCAL int getFloat
    (
    char *string,
    VALUE *pValue
    )
    {
#ifndef _WRS_NO_TGT_SHELL_FP
    pValue->side = RHS;
    pValue->type = T_DOUBLE;
    sscanf (string, "%lf", &pValue->value.dp);
#endif  /* _WRS_NO_TGT_SHELL_FP */
    return (DS_FLOAT);
    }
/*******************************************************************************
*
* getString - interpret scanned string as quoted string
*
* RETURNS: STRING
*/
LOCAL int getString
    (
    char *string,
    int nChars,
    VALUE *pValue
    )
    {
    pValue->side = RHS;
    pValue->type = T_INT;
    string [nChars - 1] = EOS;
    pValue->value.rv = (ULONG)addTempString (&string[1]);
    return (STRING);
    }
/*******************************************************************************
*
* getChar - interpret scanned string as quoted character
*
* RETURNS: char
*/
LOCAL int getChar
    (
    char *string,
    int nChars,
    VALUE *pValue
    )
    {
    char ch;
    int n = strToChar (&string [1], &ch);
    if (nChars != (n + 2))
    {
    lexError (string, "invalid char"); 
    return (LEX_ERROR);
    }
    pValue->side       = RHS;
    pValue->type       = T_BYTE;
    pValue->value.byte = ch;
    return (DS_CHAR);
    }
/*******************************************************************************
*
* getId - interpret scanned string as identifier or keyword
*
* RETURNS: TYPECAST, {T,D,U}_SYMBOL
*/
LOCAL int getId
    (
    char *string,
    FAST VALUE *pValue
    )
    {
    char     tempString [MAX_SHELL_LINE + 1];
    SYM_TYPE type;
    char     *value;
    int      t = typeCast (string);
    if (t != ERROR)
    {
    pValue->type = (TYPE)t;
    return (TYPECAST);
    }
    tempString[0] = '_';
    strncpy (&tempString[1], string, MAX_SHELL_LINE);
    tempString [MAX_SHELL_LINE] = EOS;
    if ((symFindByName (&tempString[1], &value, &type) == OK) ||
        (symFindByName (&tempString[0], &value, &type) == OK))
    {
    pValue->value.lv = (int64_t *) value;
    pValue->type     = T_INT;
    pValue->side     = LHS;
    if ((type & 0xe) == N_TEXT) /* only need to check three bits of type*/
        return (T_SYMBOL);
    else
        return (D_SYMBOL);
    }

    /* identifier not found */
    
    pValue->side = RHS;
    pValue->type = T_UNKNOWN;
    pValue->value.rv = (ULONG)addTempString (string);
    return (U_SYMBOL);
    }
/* HIDDEN */
typedef struct
    {
    char *string;
    int numOccurrences;
    SYM_TYPE type;
    int value;
    } ADA_MATCH;
/* END HIDDEN */

/*******************************************************************************
*
* typeCast - determine if string is a keyword type cast
*
* RETURNS: T_{BYTE,WORD,INT,FLOAT,DOUBLE}, or ERROR
*/
LOCAL int typeCast
    (
    FAST char *string
    )
    {
    static char *typen [] =
#ifndef _WRS_NO_TGT_SHELL_FP
    {"char", "SHORT", "INT", "ULONG", "FLOAT", "DOUBLE"};
#else   /* _WRS_NO_TGT_SHELL_FP */
    {"char", "SHORT", "INT", "ULONG"};
#endif  /* _WRS_NO_TGT_SHELL_FP */
    static TYPE  typet [] =
#ifndef _WRS_NO_TGT_SHELL_FP
    {T_BYTE, T_WORD, T_INT, T_INT, T_FLOAT, T_DOUBLE};
#else   /* _WRS_NO_TGT_SHELL_FP */
    {T_BYTE, T_WORD, T_INT, T_INT};
#endif  /* _WRS_NO_TGT_SHELL_FP */
    FAST int ix;
    for (ix = 0; ix < NELEMENTS (typet); ix++)
    {
    if (strcmp (string, typen [ix]) == 0)
        return ((int)typet [ix]);
    }
    return (ERROR);
    }
/*******************************************************************************
*
* strToChar - get a possibly escaped character from a string
*
* RETURNS: number of characters parsed and character in <pChar>.
*/
LOCAL int strToChar
    (
    FAST char *string,
    char *pChar
    )
    {
    FAST int nchars = 1;
    int num;
    FAST char ch;
    if (*string != '\\')
    {
    *pChar = *string;
    return (nchars);
    }
    string++;
    if ((*string >= '0') && (*string <= '7'))
    {
    sscanf (string, "%o", &num);
    ch = num % 0400;
    while ((*string >= '0') && (*string <= '7'))
        {
        ++string;
        ++nchars;
        }
    }
    else
    {
    nchars++;
    switch (*string)
        {
        case 'n':  ch = '\n'; break;
        case 't':  ch = '\t'; break;
        case 'b':  ch = '\b'; break;
        case 'r':  ch = '\r'; break;
        case 'f':  ch = '\f'; break;
        case '\\': ch = '\\'; break;
        case '\'': ch = '\''; break;
        case '"':  ch = '"'; break;
        case 'a':  ch = (char)0x07; break;
        case 'v':  ch = (char)0x0b; break;
        default:   ch = *string; break;
        }
    }
    *pChar = ch;
    return (nchars);
    }
/* lexeme scan routines */
#define EMPTY       -2
LOCAL int retractChar;
LOCAL int lastChar;
/*******************************************************************************
*
* lexInit - initialize lex scan routines
*
* RETURNS: N/A
*/
LOCAL void lexInit (void)
    {
    retractChar = EMPTY;
    }
/*******************************************************************************
*
* lexScan - scan input for next lexeme
*
* RETURNS: next lexeme.
*/
LOCAL int64_t lexScan (void)
    {
    FAST int ch;
    FAST int state;
    int nChars;
    int64_t code;
    BOOL scanContinue;
    char string [MAX_SHELL_LINE + 1];
    do
    {
    /* get first character; use any retracted character first */
    if (retractChar != EMPTY)
        {
        ch = retractChar;
        retractChar = EMPTY;
        }
    else
        ch = *(nextChar++);
    /* consume characters until final state reached */
    state = 0;
    for (nChars = 0; nChars < MAX_SHELL_LINE; nChars++)
        {
        /* consume character and make state transition */
        string [nChars] = ch;
        state = lexStateTable [state * lexNclasses + lexClass [ch + 1]];
        /* if final state reached, quit; otherwise get next character */
        if (state < 0)
        {
        nChars++;
        break;
        }
        ch = *(nextChar++);
        }
    /* final state reached */
    state = -state;
    string [nChars] = EOS;
    lastChar = ch;
    code = lexActions (state, string, nChars, &scanContinue);
    }
    while (scanContinue);
    return (code);
    }
/*******************************************************************************
*
* lexRetract - retract last character consumed
*
* RETURNS: N/A
*/
LOCAL void lexRetract (void)
    {
    retractChar = lastChar;
    }
    

