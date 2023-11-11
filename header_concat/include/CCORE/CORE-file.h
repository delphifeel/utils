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
