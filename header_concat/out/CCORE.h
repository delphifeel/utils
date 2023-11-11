#ifndef _CCORE_H_
#define _CCORE_H_


/*
*            CORE-types.h
*/
#ifndef _CORE_TYPES_H_
#define _CORE_TYPES_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef unsigned   uint;
typedef uint8_t    uint8;                              
typedef uint16_t   uint16;
typedef uint32_t   uint32;
typedef uint64_t   uint64;
typedef int8_t     int8;                               
typedef int16_t    int16;
typedef int32_t    int32;
typedef int64_t    int64;


#endif

/*
*            CORE-debug.h
*/
#ifndef _CORE_DEBUG_H_
#define _CORE_DEBUG_H_

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


/**
 *  ___________________________________________________________________________________________________________
 * |																										   |																									   
 * |												CONSTANTS 												   |
 * |___________________________________________________________________________________________________________| 
 * 
 */

#ifdef CORE_SET_TESTING_ENABLED
	#define CORE_TESTING_ENABLED
#endif

#ifdef CORE_SET_UNSAFE_RELEASE
	#define CORE_UNSAFE_RELEASE
#endif

#ifndef CORE_SET_DEBUG_COLORED
	#define _COREDEBUG_FUNC_SYMBOL			("")
	#define _COREDEBUG_INFO_SYMBOL			("")
	#define _COREDEBUG_RESET_SYMBOL			("")
	#define _COREDEBUG_ERROR_SYMBOL			("")
	#define _COREDEBUG_WARN_SYMBOL			("")
	#define _COREDEBUG_FILE_SYMBOL			("")
#else
	#define _COREDEBUG_COLOR_SYMBOL(_COLOR)		("\033[1;" #_COLOR "m")
	#define _COREDEBUG_RESET_SYMBOL				("\033[0m")
	#define _COREDEBUG_ERROR_SYMBOL				(_COREDEBUG_COLOR_SYMBOL(31))
	#define _COREDEBUG_WARN_SYMBOL				(_COREDEBUG_COLOR_SYMBOL(35))
	#define _COREDEBUG_INFO_SYMBOL				(_COREDEBUG_COLOR_SYMBOL(36))
	#define _COREDEBUG_FUNC_SYMBOL				(_COREDEBUG_COLOR_SYMBOL(34))
	#define _COREDEBUG_FILE_SYMBOL				(_COREDEBUG_COLOR_SYMBOL(04))
#endif

#define CORE_DebugStdOut(...) 	fprintf(stdout, __VA_ARGS__)
#define CORE_DebugStdErr(...) 	fprintf(stderr, __VA_ARGS__)


// static const char *_CORE_module_name = NULL;
// #define CORE_GetModuleName(file_name, module_name)
// {
// 	if (module_name == NULL)
// 	{
// 		const char *temp_ptr = file_name;
// 		while (*temp_ptr != 0)
// 		{
// 			if ((*temp_ptr == '/') ||
// 				(*temp_ptr == '\\'))
// 			{
// 				module_name = temp_ptr;
// 			}

// 			temp_ptr++;
// 		}

// 		if (module_name == NULL)
// 		{
// 			module_name = file_name;
// 		}
// 		else
// 		{
// 			module_name++;
// 		}
// 	}

// 	return module_name;
// }

#define _CORE_DEBUG_MESSAGE_PRE(_TYPE, _TYPE_SYMBOL, _FILE, _LINE)				\
	"%s[%s]%s %s(%s:%d)%s %s%s()%s ", 											\
	(_TYPE_SYMBOL), 															\
	(_TYPE), 																	\
	_COREDEBUG_RESET_SYMBOL,													\
	_COREDEBUG_FILE_SYMBOL,														\
	_FILE,	                                            						\
	_LINE, 																		\
	_COREDEBUG_RESET_SYMBOL,													\
	_COREDEBUG_FUNC_SYMBOL,														\
	__func__,																	\
	_COREDEBUG_RESET_SYMBOL														

#define _CORE_DEBUG_PRINT(_TYPE, _TYPE_SYMBOL) 			\
	(CORE_DebugStdOut(_CORE_DEBUG_MESSAGE_PRE(_TYPE, _TYPE_SYMBOL, __FILE__, __LINE__)))

#define _CORE_DEBUG_ERROR(_TYPE, _TYPE_SYMBOL) 			\
	(CORE_DebugStdErr(_CORE_DEBUG_MESSAGE_PRE(_TYPE, _TYPE_SYMBOL, __FILE__, __LINE__)))

#define _CORE_DEBUG_ERROR_EX(_TYPE, _TYPE_SYMBOL, _FILE, _LINE) 			\
	(CORE_DebugStdErr(_CORE_DEBUG_MESSAGE_PRE(_TYPE, _TYPE_SYMBOL, _FILE, _LINE)))


/**
 *  ___________________________________________________________________________________________________________
 * |																										   |																									   
 * |												ASSERTS 												   |
 * |___________________________________________________________________________________________________________| 
 * 
 */
#ifdef CORE_UNSAFE_RELEASE
	#define CORE_Assert(EXPRESSION)  ((void) 0)						
	#define CORE_AssertWithMessage(EXPRESSION, ...)  ((void) 0)	

	#define CORE_AssertWithMessageEx(FILE, LINE, EXPRESSION, ...) ((void) 0)

	#define CORE_AssertPointer(PTR)     ((void) 0)
	#define CORE_AssertIntEqual(A, B) 	((void) 0)

	#define CORE_Abort(...) 	( abort() )
#else
	#define CORE_Assert(EXPRESSION) 						(	(EXPRESSION) ? (void) true : (_CORE_DEBUG_ERROR("ASSERT FAILED", _COREDEBUG_ERROR_SYMBOL), CORE_DebugStdErr("%s\n", #EXPRESSION), abort())	)
	#define CORE_AssertWithMessage(EXPRESSION, ...) 		(	(EXPRESSION) ? (void) true : (_CORE_DEBUG_ERROR("ASSERT FAILED", _COREDEBUG_ERROR_SYMBOL), CORE_DebugStdErr(__VA_ARGS__), abort())	)

	#define CORE_AssertWithMessageEx(FILE, LINE, EXPRESSION, ...) \
		(	(EXPRESSION) ? (void) true : (_CORE_DEBUG_ERROR_EX("ASSERT FAILED", _COREDEBUG_ERROR_SYMBOL, FILE, LINE), CORE_DebugStdErr(__VA_ARGS__), abort())	)

	#define CORE_AssertPointer(PTR) 						(	CORE_AssertWithMessage((PTR) != NULL, "`%s` is NULL\n", #PTR)	)
	#define CORE_AssertIntEqual(A, B) 						( CORE_AssertWithMessage(A == B, #A " == " #B " (%d vs %d)\n", A, B) )

	#define CORE_Abort(...) 								(	_CORE_DEBUG_ERROR("ABORT", _COREDEBUG_ERROR_SYMBOL), CORE_DebugStdErr(__VA_ARGS__), abort()	)
#endif

/**
 *  ___________________________________________________________________________________________________________
 * |																										   |																									   
 * |											DEBUG LEVEL 												   |
 * |___________________________________________________________________________________________________________| 
 * 
 */

#define CORE_DEBUG_LEVEL_NONE		(0)
#define CORE_DEBUG_LEVEL_ERROR		(1)
#define CORE_DEBUG_LEVEL_WARNING	(2)
#define CORE_DEBUG_LEVEL_INFO		(3)

#define CORE_DEFAULT_DEBUG_LEVEL 		(CORE_DEBUG_LEVEL_INFO)

#if defined(CORE_TESTING_ENABLED)
	#define CORE_DEBUG_LEVEL 			(CORE_DEBUG_LEVEL_NONE)
#elif !defined(CORE_TESTING_ENABLED) && defined(CORE_SET_DEBUG_LEVEL)
	#define CORE_DEBUG_LEVEL 			(CORE_SET_DEBUG_LEVEL) 			
#else
	#define CORE_DEBUG_LEVEL 			(CORE_DEFAULT_DEBUG_LEVEL)
#endif

#if CORE_DEBUG_LEVEL >= CORE_DEBUG_LEVEL_INFO
	#define CORE_DebugInfo(...) 	(	_CORE_DEBUG_PRINT("INFO", _COREDEBUG_INFO_SYMBOL), 		CORE_DebugStdOut(__VA_ARGS__)	)
#else
	#define CORE_DebugInfo(...) 	((void) 0)
#endif

#if CORE_DEBUG_LEVEL >= CORE_DEBUG_LEVEL_WARNING
	#define CORE_DebugWarning(...) 	(	_CORE_DEBUG_PRINT("WARNING", _COREDEBUG_WARN_SYMBOL), 	CORE_DebugStdOut(__VA_ARGS__)	)
#else
	#define CORE_DebugWarning(...) 	((void) 0)
#endif

#if CORE_DEBUG_LEVEL >= CORE_DEBUG_LEVEL_ERROR
	#define CORE_DebugError(...) 	(	_CORE_DEBUG_ERROR("ERROR", _COREDEBUG_ERROR_SYMBOL), 		CORE_DebugStdErr(__VA_ARGS__)	)
#else
	#define CORE_DebugError(...) 	((void) 0)
#endif


#endif

/*
*            CORE-memory.h
*/
#ifndef _CORE_MEMORY_H_
#define _CORE_MEMORY_H_

#include <stdlib.h>
#include <stddef.h>
#include <string.h>


#define CORE_MemAlloc(SIZE, COUNT)          ( malloc(COUNT * SIZE) )
#define CORE_MemCalloc(SIZE, COUNT)         ( calloc(COUNT, SIZE) )
#define CORE_MemRealloc(PTR, SIZE, COUNT)   ( realloc(PTR, SIZE * COUNT) )

#define CORE_MemFree(PTR)                                                               \
  CORE_AssertWithMessageEx(__FILE__, __LINE__, PTR != NULL, "Double free\n");           \
  free(PTR);                                                                            \
  PTR = NULL;

#define CORE_MemSet 				                memset
#define CORE_MemZero(PTR, SIZE)		                CORE_MemSet(PTR, 0, SIZE)

#define CORE_MemCpy                                 memcpy
#define CORE_MemCmp(A, B, SIZE)                     (memcmp(A, B, SIZE) == 0)
#define CORE_MemEqual(PTR1, PTR2, SIZE)             (memcmp(PTR1, PTR2, SIZE) == 0)

#define CORE_MemNewCopy(DEST, SRC, SRC_SIZE)      \
    DEST = CORE_MemAlloc((SRC_SIZE), 1);          \
    CORE_MemCpy(DEST, SRC, (SRC_SIZE));                   
 

#endif

/*
*            CORE-string.h
*/
#ifndef _CORE_STRING_H_
#define _CORE_STRING_H_


#define CORE_StrPrintf(DEST, DEST_SIZE, ...)	(snprintf(DEST, DEST_SIZE, __VA_ARGS__))
#define CORE_StrCat(DEST, DEST_SIZE, SRC)       (strncat(DEST, SRC, (DEST_SIZE - 1)))
#define CORE_StrCpy(DEST, DEST_SIZE, SRC)       (CORE_StrPrintf(DEST, DEST_SIZE, "%s", SRC))
#define CORE_StrEqual(STR1, STR2)               (strcmp(STR1, STR2) == 0)
#define CORE_StrLen(STR1)                       (strlen(STR1))
#define CORE_StrNewCopy(DEST, STR, STR_SIZE) \
    CORE_MemNewCopy(DEST, STR, STR_SIZE);    \
    DEST[STR_SIZE] = 0;
	
const char *CORE_StrFindEnd(const char *str, uint str_len, const char *substr, uint substr_len);

#ifdef CCORE_IMPL
const char *CORE_StrFindEnd(const char *str, uint str_len, const char *substr, uint substr_len)
{
    ssize_t start = -1;
    for (uint i = 0; i < str_len; i++) {
        if (str[i] == substr[0]) {
            start = i;
            break;
        }
    }
    if (start == -1) {
        return NULL;
    }

    if (0 == strncmp(str + start, substr, substr_len)) {
        return str + start + substr_len;
    }
    return NULL;
}
#endif

#endif

/*
*            CORE-file.h
*/
#ifndef _CORE_FILE_H_
#define _CORE_FILE_H_

#define CORE_FileWrite(FILE_NAME, BUFF, BUFF_SIZE) do {  \
    FILE *f = fopen(FILE_NAME,"wb");                    \
    if (!f) {                                                \
        CORE_DebugStdErr("Error open file %s\n", FILE_NAME); \
        break;                                               \
    }                                                        \
    fwrite(BUFF, BUFF_SIZE, 1, f);                          \
    fclose(f);                                             \
} while (0)
	
#define CORE_FileReadAll(FILE_NAME, BUFF_PTR, BUFF_SIZE_PTR) do {   \
    FILE *f = fopen(FILE_NAME, "rb");                               \
    if (f == NULL) {                                                \
        CORE_Abort("Can't find file to read all: %s\n", FILE_NAME); \
    }                                                               \
    fseek(f, 0, SEEK_END);                                          \
    long fsize = ftell(f);                                          \
    fseek(f, 0, SEEK_SET);                                          \
    *BUFF_PTR = CORE_MemAlloc(1, fsize + 1);                        \
	char *buff = *BUFF_PTR;                                         \
    *BUFF_SIZE_PTR = fsize + 1;                                     \
    fread(buff, *BUFF_SIZE_PTR, 1, f);                              \
    fclose(f);                                                      \
    buff[fsize] = 0;                                                \
} while (0)

#endif

/*
*            CORE-vector.h
*/
#ifndef _CORE_VECTOR_H_
#define _CORE_VECTOR_H_


#define _CVectorNewType(_TYPE)  CVector_##_TYPE
#define _CVector_SizeOfItem(_VECTOR)  (sizeof(*((_VECTOR)->array)))

#define CVector_Define(_TYPE)   \
  typedef struct {              \
    _TYPE *array;               \
    uint size;                 \
    uint cap;             \
  }  _CVectorNewType(_TYPE)

#define CVector_DefineBig(_TYPE)   \
  typedef struct {              \
    _TYPE *array;               \
    uint64  size;                 \
    uint64  cap;                  \
  }  _CVectorNewType(_TYPE)
  
#define CVector(_TYPE)  \
  _CVectorNewType(_TYPE)
  
#define CVector_Init(_VECTOR, _CAP)                                       \
  (_VECTOR)->array = CORE_MemAlloc(_CVector_SizeOfItem(_VECTOR), _CAP);   \
  CORE_AssertWithMessage((_VECTOR)->array != NULL, "Out of memory bro\n"); \
  (_VECTOR)->size = 0;                                                    \
  (_VECTOR)->cap = _CAP;
  
#define CVector_Free(_VECTOR)           \
  CORE_MemFree((_VECTOR)->array);       \
  (_VECTOR)->array = NULL;
  
#define CVector_Push(_VECTOR, _VALUE)         \
  CORE_AssertWithMessage(                     \
    (_VECTOR)->array != NULL,                 \
    "CVector array is NULL. You forgot to call CVector_Init ?\n"  \
  );                                          \
  if ((_VECTOR)->size == (_VECTOR)->cap) {    \
    (_VECTOR)->cap = (_VECTOR)->cap * 2 + 1;  \
    (_VECTOR)->array = CORE_MemRealloc(       \
      (_VECTOR)->array,                       \
      _CVector_SizeOfItem(_VECTOR),           \
      (_VECTOR)->cap                          \
    );                                        \
  }                                           \
  (_VECTOR)->array[(_VECTOR)->size++] = _VALUE; 

#define _CVector_GetPtr(_VECTOR, _INDEX)                                        \
  (                                                                             \
    CORE_AssertWithMessage(_INDEX < (_VECTOR)->size, "Index out of bounds\n"),  \
    (_VECTOR)->array + _INDEX                                                   \
  )        

#define CVector_GetPtr(_VECTOR, _INDEX) \
  (_CVector_GetPtr(_VECTOR, _INDEX))

#define CVector_Get(_VECTOR, _INDEX) \
  (                                                                             \
    CORE_AssertWithMessage(_INDEX < (_VECTOR)->size, "Index out of bounds\n"),  \
    (_VECTOR)->array[_INDEX]                                                    \
  )  

#define CVector_Pop(_VECTOR)                                                  \
  (                                                                           \
    CORE_AssertWithMessage((_VECTOR)->size > 0, "Vector is empty\n"),         \
    (_VECTOR)->size--,                                                        \
    (_VECTOR)->array[(_VECTOR)->size]                                         \
  )
  

#define CVector_Print(_VECTOR, _FORMAT)         \
{                                               \
  printf("[");                                  \
  for (uint i = 0; i < (_VECTOR)->size; i++) {  \
    printf(_FORMAT, (_VECTOR)->array[i]);       \
    printf(", ");                               \
  }                                             \
  printf("]");                                  \
}

#endif

#endif
