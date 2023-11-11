#define CCORE_IMPL
#include "include/CCORE.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

static bool verbose = false;

#define print_verbose(...)  ( verbose ? (CORE_DebugStdOut(__VA_ARGS__), (void) 0) : (void) 0 )

const char *CORE_StrFindInside(const char *str, uint str_len, const char *tokens[], uint tokens_len, uint *buff_len)
{
    const char *str_end = str + str_len;
    for (uint i = 0; i < tokens_len - 1; i++) {
        str = CORE_StrFindEnd(str, str_end - str, tokens[i], CORE_StrLen(tokens[i]));
        if (str == NULL) {
            return NULL;
        }
    }

    const char *last_token = tokens[tokens_len - 1];
    const char *str_before_last = CORE_StrFindEnd(str, str_end - str, last_token, CORE_StrLen(last_token));
    if (str_before_last == NULL) {
        return NULL;
    }
    str_before_last -= 1;
    *buff_len = str_before_last - str;
    return str;
}

void embed_file(const char *full_path, FILE *file_descriptor)
{
    FILE *fp = fopen(full_path, "r");
    if (fp == NULL) {
        CORE_Abort("Can't find file %s\n", full_path);
    }

    size_t len = 0;
    size_t read = 0;
    char *line = NULL;

    const char *filename = basename((char *) full_path);
    char file_prefix[255];
    CORE_StrPrintf(file_prefix, sizeof(file_prefix), "\n/*\n*            %s\n*/\n", filename);
    
    fwrite(file_prefix, CORE_StrLen(file_prefix), 1, file_descriptor);

    const char *tokens[] = {"#include", "\""};
    uint tokens_len = 2;
    uint buff_len = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        const char *str = CORE_StrFindInside(line, read, tokens, tokens_len, &buff_len);
        if (str == NULL) {
            fwrite(line, read, 1, file_descriptor);
        }
    }
    print_verbose("Embeding content of %s\n", full_path);

    if (line != NULL) {
        free(line);
    }
}

// we are searching for #include and replace it with actual file content
// in other cases we just copy-paste
bool process_line(const char *line, uint line_size, FILE *file_descriptor, const char *dir)
{
    uint file_to_embed_len = 0;
    const char *tokens[] = {"#include", "\"", "\""};
    uint tokens_len = 3;
    const char *file_to_embed = CORE_StrFindInside(line, line_size, tokens, tokens_len, &file_to_embed_len);
    if (file_to_embed == NULL) {
        return false;
    }

    char *file_name = NULL;
    CORE_StrNewCopy(file_name, file_to_embed, file_to_embed_len);

    char full_path[255];
    CORE_StrPrintf(full_path, sizeof(full_path), "%s/%s", dir, file_name);
    embed_file(full_path, file_descriptor);

    return true;
}

int main(int argc, const char **argv) 
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    if (argc < 3) {
        CORE_Abort("Wrong usage. Format: ./bin [file_to_read] [file_to_write] v?\n");
    }

    const char *file_to_read = argv[1];
    const char *file_to_write = argv[2];
    if (argc == 4 && argv[3][0] == 'v') {
        verbose = true;
    }

    fp = fopen(file_to_read, "r");
    if (fp == NULL) {
        CORE_Abort("Can't find file to read: %s\n", file_to_read);
    }
    print_verbose("Analysing %s\n", file_to_read);

    remove(file_to_write);
    print_verbose("Removing %s\n", file_to_write);
    
    FILE *file_descriptor = fopen(file_to_write, "wb");
    if (file_descriptor == NULL) {
        CORE_Abort("Can't find file to write: %s\n", file_to_write);
    }
    const char *dir = dirname((char *) file_to_read);
    while ((read = getline(&line, &len, fp)) != -1) {
        if (!process_line(line, read, file_descriptor, dir)) {
            fwrite(line, read, 1, file_descriptor);
        }
    }

    fclose(file_descriptor);
    fclose(fp);
    if (line)
        free(line);
    print_verbose("Done!\n");
    exit(EXIT_SUCCESS);
}
