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

char *CORE_FileReadAll(const char *filename, size_t *buff_size);
	
#ifdef CCORE_IMPL
char *CORE_FileReadAll(const char *filename, size_t *buff_size)
{
    FILE *f = fopen(filename, "rb");                               
    if (f == NULL) {                                                
       return NULL;
    }                                                               
    fseek(f, 0, SEEK_END);                                          
    long fsize = ftell(f);                                          
    fseek(f, 0, SEEK_SET);                                         
    char *buff = CORE_MemAlloc(1, fsize + 1);                        
    *buff_size = fsize + 1;                                     
    fread(buff, *buff_size, 1, f);                              
    fclose(f);                                                      
    buff[fsize] = 0;                                                
    return buff;
}
#endif

#endif
