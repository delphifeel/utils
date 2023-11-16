#ifndef _CORE_STRING_H_
#define _CORE_STRING_H_

#include "CORE-types.h"

// --> Macroses
#define CORE_StrPrintf(DEST, DEST_SIZE, ...)	(snprintf(DEST, DEST_SIZE, __VA_ARGS__))
#define CORE_StrCat(DEST, DEST_SIZE, SRC)       (strncat(DEST, SRC, (DEST_SIZE - 1)))
#define CORE_StrCpy(DEST, DEST_SIZE, SRC)       (CORE_StrPrintf(DEST, DEST_SIZE, "%s", SRC))
#define CORE_StrEqual(STR1, STR2)               (strcmp(STR1, STR2) == 0)
#define CORE_StrLen(STR1)                       (strlen(STR1))
#define CORE_StrNewCopy(DEST, STR, STR_SIZE) \
    CORE_MemNewCopy(DEST, STR, STR_SIZE);    \
    DEST[STR_SIZE] = 0;

// Macroses -->


typedef struct {
    const char *str;
    uint len;
} CMatch;


// --> Definition
const char *CORE_StrFindEnd(const char *str, uint str_len, const char *substr, uint substr_len);
void CORE_StrDelSpaces(char *str, uint str_len);
uint CORE_StrFindInside(const char *str, uint str_len, const char *expr, CMatch matches[], uint matches_size);
// Definition -->


// --> Implementation
#ifdef CCORE_IMPL
const char *CORE_StrFindEnd(const char *str, uint str_len, const char *substr, uint substr_len)
{
    const char *str_pos = str;
    while (*str_pos != 0) {
        if (0 == strncmp(str_pos, substr, substr_len)) {
            return str_pos + substr_len;
        }
        str_pos++;
    }
    return NULL;
}

void CORE_StrDelSpaces(char *str, uint str_len)
{
    char buff[str_len+1];
    buff[0] = 0;

    char *token = strtok(str, " ");
    CORE_StrCat(buff, sizeof(buff), token);
    while (1) {
        token = strtok(NULL, " ");
        if (token == NULL) 
            break;
        CORE_StrCat(buff, sizeof(buff), token);
    }
    CORE_StrCpy(str, str_len, buff);
}

// ex: `#include    "*"   ` means get 1 match inside ""
uint CORE_StrFindInside(const char *str, uint str_len, const char *expr, CMatch matches[], uint matches_size)
{
    uint expr_len = CORE_StrLen(expr);
    char expr_mut[expr_len + 1];
    CORE_StrCpy(expr_mut, sizeof(expr_mut), expr);

    // delete spaces: `#include"*"`
    //CORE_StrDelSpaces(expr_mut, expr_len);
    
    uint matches_count = 0;
    // use * as token: `#include"`, `"`
    char *token = strtok(expr_mut, "*");
    if (token == NULL) {
        return 0;
    }
    const char *str_end = str + str_len;
    // TODO: need func for ignoring whitespaces when searching
    str = CORE_StrFindEnd(str, str_len, token, CORE_StrLen(token));
    if (str == NULL) {
        return 0;
    }
    const char *str_start = str;
    
    while (1) {
        token = strtok(NULL, "*");
        if (token == NULL) 
            return matches_count;

        
        str = CORE_StrFindEnd(str, str_end - str, token, CORE_StrLen(token));
        if (str == NULL) {
            return matches_count;
        }
        if (matches_size == matches_count) {
            return matches_count;
        }

        matches[matches_count].str = str_start;
        matches[matches_count].len = str - 1 - str_start;
        matches_count++;

        str_start = str;
    }

    return matches_count;
}
#endif
// Implementation -->

#endif
