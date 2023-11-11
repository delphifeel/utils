#ifndef _CORE_VECTOR_H_
#define _CORE_VECTOR_H_

#include "CORE-memory.h"
#include "CORE-debug.h"

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
