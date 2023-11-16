#define CCORE_IMPL
#include "include/CCORE.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

static bool verbose = false;

#define print_verbose(...)  ( verbose ? (CORE_DebugStdOut(__VA_ARGS__), (void) 0) : (void) 0 )

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

    CMatch matches[1];
    while ((read = getline(&line, &len, fp)) != -1) {
        uint matches_count = CORE_StrFindInside(line, read, "#include \"*\"", matches, 1);
        if (matches_count == 0) {
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
    CMatch matches[1];
    uint matches_count = CORE_StrFindInside(line, line_size, "#include \"*\"", matches, 1);
    if (matches_count == 0) {
        return false;
    }
    const char *file_to_embed = matches[0].str;
    uint file_to_embed_len = matches[0].len;

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
