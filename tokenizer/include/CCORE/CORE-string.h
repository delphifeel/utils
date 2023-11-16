#ifndef _CORE_STRING_H_
#define _CORE_STRING_H_

#include "CORE-debug.h"
#include "CORE-memory.h"
#include "CORE-types.h"
#include "CORE-records.h"
#include <ctype.h>

// --> Macroses
#define CORE_StrPrintf(DEST, DEST_SIZE, ...)	(snprintf(DEST, DEST_SIZE, __VA_ARGS__))
#define CORE_StrCpy(DEST, DEST_SIZE, SRC)       (CORE_StrPrintf(DEST, DEST_SIZE, "%s", SRC))
#define CORE_StrCat(DEST, DEST_SIZE, DEST_LEN, SRC)  \
    DEST_LEN += CORE_StrCpy((DEST) + (DEST_LEN), (DEST_SIZE) - (DEST_LEN), SRC);  \
    if (((DEST_LEN) + 1) > (DEST_SIZE))   DEST_LEN = DEST_SIZE - 1;

#define CORE_StrEqual(STR1, STR2)     \
    (CORE_AssertPointer(STR1), CORE_AssertPointer(STR2), (strcmp(STR1, STR2) == 0))
#define CORE_StrLen(STR1)                       (strlen(STR1))
#define CORE_StrNewCopy(DEST, STR, STR_SIZE) \
    CORE_MemNewCopy(DEST, STR, STR_SIZE);    \
    DEST[STR_SIZE] = 0;

#define CORE_BuffPrint(STR, STR_LEN) do {    \
    printf("[");                            \
    for (uint i = 0; i < (STR_LEN); i++) {  \
        printf("%c", (STR)[i]);     \
    }                                       \
    printf("]");                            \
} while (0)

#define CORE_BuffInit(BUFF_NAME, SRC_STR)           \
    char BUFF_NAME[CORE_StrLen(SRC_STR) + 1];       \
    CORE_MemCpy(BUFF_NAME, SRC_STR, sizeof(BUFF_NAME) - 1);  \
    BUFF_NAME[sizeof(BUFF_NAME) - 1] = 0

// Macroses -->


typedef struct {
    const char *str;
    uint len;
} CMatch;


// --> Definition
bool CORE_CharOneOf(char c, const char *chars, uint chars_len);
char *CORE_StrTrim(char *str);
uint CORE_StrSplit(char *str, const char *separators, char *tokens[], uint tokens_size);
CRecords CORE_StrSplitToRecords(char *str, const char *fields_separators, char record_separator);
const char *CORE_StrFindEnd(const char *str, uint str_len, const char *substr, uint substr_len);
void CORE_StrDelSpaces(char *str, uint str_len);
//uint CORE_StrFindInside(const char *str, uint str_len, const char *expr, CMatch matches[], uint matches_size);
bool CORE_StrOneOf(const char *str, const char **list, uint list_len);
// Definition -->


// --> Implementation
#ifdef CCORE_IMPL
uint CORE_StrSplit(char *str, const char *separators, char *tokens[], uint tokens_size)
{
    CORE_AssertPointer(str);
    CORE_MemZero(tokens, sizeof(char *) * tokens_size);
    uint tokens_len = 0;
    char *token = strtok(str, separators);
    tokens[tokens_len++] = token;

    while (tokens_len < tokens_size) {
        token = strtok(NULL, separators);
        if (token == NULL) {
            break;
        }
        tokens[tokens_len++] = token;
    }
    return tokens_len;
}

#define _STR_SPLIT_DEF_VEC_SIZE (10)
CRecords CORE_StrSplitToRecords(char *str, const char *fields_separators, char record_separator)
{
    CORE_AssertPointer(str);
    CVector(CRecord) records;
    CVector_Init(&records, _STR_SPLIT_DEF_VEC_SIZE);

    uint fields_separators_count = CORE_StrLen(fields_separators);
    CVector(string) curr_fields;
    CVector_Init(&curr_fields, _STR_SPLIT_DEF_VEC_SIZE);
    CVector_Push(&curr_fields, (str));
    char c = 0;
    while ((c = *str) != 0) {
        if (CORE_CharOneOf(c, fields_separators, fields_separators_count)) {
            *str = '\0';
            char **curr_field = CVector_LastPtr(&curr_fields);
            if ((str - *curr_field) > 0) {
                CVector_Push(&curr_fields, (str + 1));
            } else {
                (*curr_field)++;
            }
            str++;
            continue;
        }
        if (c == record_separator) {
            *str = '\0';
            CRecord record = {curr_fields};
            CVector_Push(&records, record);
            CVector_Init(&curr_fields, _STR_SPLIT_DEF_VEC_SIZE);
            CVector_Push(&curr_fields, (str + 1));
            str++;
            continue;
        }
        str++;
    }
    CVector_Push(&curr_fields, (str + 1));
    CRecord record = {curr_fields};
    CVector_Push(&records, record);

    return records;
}

bool CORE_CharOneOf(char c, const char *chars, uint chars_len)
{
    for (uint i = 0; i < chars_len; i++) {
        if (c == chars[i]) {
            return true;
        }
    }
    return false;
}

bool CORE_StrOneOf(const char *str, const char **list, uint list_len)
{
    for (uint i = 0; i < list_len; i++) {
        if (CORE_StrEqual(str, list[i])) {
            return true;
        }
    }
    return false;
}

const char *CORE_StrFindEnd(const char *str, uint str_len, const char *substr, uint substr_len)
{
    CORE_AssertPointer(str);
    const char *str_pos = str;
    while (*str_pos != 0) {
        if (0 == strncmp(str_pos, substr, substr_len)) {
            return str_pos + substr_len;
        }
        str_pos++;
    }
    return NULL;
}

char *CORE_StrTrim(char *str)
{
    CORE_AssertPointer(str);
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

void CORE_StrDelSpaces(char *str, uint str_len)
{
    CORE_AssertPointer(str);
    char buff[str_len+1];
    buff[0] = 0;
    uint buff_len = 0;

    char *token = strtok(str, " ");
    CORE_StrCat(buff, sizeof(buff), buff_len, token);
    while (1) {
        token = strtok(NULL, " ");
        if (token == NULL) 
            break;
        CORE_StrCat(buff, sizeof(buff), buff_len, token);
    }
    CORE_StrCpy(str, str_len, buff);
}

// ex: `#include    "*"   ` means get 1 match inside ""
// TODO (delphifeel): it doesn't work as intended
/*uint CORE_StrFindInside(const char *str, uint str_len, const char *expr, CMatch matches[], uint matches_size)
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

        uint token_len = CORE_StrLen(token);
        str = CORE_StrFindEnd(str, str_end - str, token, token_len);
        if (str == NULL) {
            return matches_count;
        }
        if (matches_size == matches_count) {
            return matches_count;
        }

        matches[matches_count].str = str_start;
        matches[matches_count].len = str - token_len - str_start;
        matches_count++;

        str_start = str;
    }

    return matches_count;
}*/
#endif
// Implementation -->

#endif
