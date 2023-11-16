#ifndef _CORE_RECORDS_H_
#define _CORE_RECORDS_H_

#include "CORE-vector.h"

typedef struct {
    CVector(string) fields;
} CRecord;

CVector_Define(CRecord);
typedef CVector(CRecord) CRecords;

#define CRecords_Free(records_ptr)  do { \
    for (uint i = 0; i < (records_ptr)->size; i++) {                  \
        CVector_Free(&CVector_GetPtr((records_ptr), i)->fields);      \
    }                                                           \
    CVector_Free((records_ptr));                                      \
} while (0)


#define CRecords_GetField(record_ptr, index)  (CVector_Get(&record_ptr->fields, index))
#define CRecords_Get(records_ptr, index)      (CVector_GetPtr(records_ptr, index))


void CRecords_TrimFields(CRecords *records);


#ifdef CCORE_IMPL
char *CORE_StrTrim(char *str);

void CRecords_TrimFields(CRecords *records)
{
    for (uint i = 0; i < records->size; i++) {
        CRecord *r = CRecords_Get(records, i);
        for (uint j = 0; j < r->fields.size; j++) {
            char **field_ptr = CVector_GetPtr(&r->fields, j);
            *field_ptr = CORE_StrTrim(*field_ptr);
        }
    }
}
#endif

#endif
