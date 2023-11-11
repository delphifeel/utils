#ifndef _CORE_MEMORY_H_
#define _CORE_MEMORY_H_

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "CORE-debug.h"
#include "CORE-types.h"


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
