#include <stdio.h>
#include <string.h>
#define CCORE_IMPL
#include "include/CCORE.h"

static char     *buff = NULL;
static size_t    buff_len = 0;
static bool      buff_records_set = false;
static CRecords  buff_records;
#define COMMAND_TOKENS_SIZE (20)
static char      command_copy[255];
static char     *command_args[COMMAND_TOKENS_SIZE];

static void split_command_to_args(const char *command)
{
    CORE_StrCpy(command_copy, sizeof(command_copy), command);
    CORE_StrSplit(command_copy, " ", command_args, COMMAND_TOKENS_SIZE);
}

static void buff_records_clear()
{
    if (buff_records_set) {
        CRecords_Free(&buff_records);
    }
    buff_records_set = false;
}

static void buff_set(char *v, size_t v_len)
{
    if ((v == NULL) && (buff != NULL)) {
        CORE_MemFree(buff);
    }
    buff_records_clear();

    buff = v;
    buff_len = v_len;
}

static char symb_to_char(char symb) {
    switch (symb) {
        case 's':
            return ' ';
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case '"':
            return '\"';
        default:
            return symb;
    }
}

static char *parse_str_quotas(char *str)
{
    if (str == NULL) {
        return NULL;
    }
    
    uint str_len = CORE_StrLen(str);

    const char *str_end = str + str_len;
    char *str_left = (char *) CORE_StrFindEnd(str, str_end - str, "\"", 1);
    if (str_left == NULL) {
        return NULL;
    }
    char *str_right = (char *) CORE_StrFindEnd(str_left, str_end - str_left, "\"", 1);
    if (str_right == NULL) {
        return NULL;
    }
    str_right--;
    *str_right = '\0';

    char buff[str_len + 1];
    buff[0] = 0;
    uint buff_len = 0;

    char c = 0;
    while ((c = *str_left++) != '\0') {
        if ((*str_left != '\0') && (c == '\\')) {
            c = symb_to_char(*str_left);
            str_left++;
        }

        buff[buff_len++] = c;
    }
    buff[buff_len] = '\0';
    CORE_StrCpy(str, str_len, buff);
    return str;
}

static void process_read()
{
    const char *filename = command_args[1];
    if (filename == NULL) {
        printf("Wrong READ usage. Format: r [filename]\n");
        return;
    }
    char *file_content = CORE_FileReadAll(filename, &buff_len);
    if (file_content == NULL) {
        printf("ERROR: file not exist `%s`\n", filename);
        return;
    }

    buff_set(file_content, buff_len);
}

static void print_buff()
{
    if (buff == NULL) {
        printf("BUFFER is empty\n");
        return;
    }
    if (!buff_records_set) {
        const char *full = command_args[1];
        if ((full != NULL) && (CORE_StrEqual(full, "full"))) {
            printf("BUFFER[%ld]:\n%s\n\n", buff_len, buff);
            return;
        }

        printf("BUFFER size = %ld\n", buff_len);
        return;
    }


    printf("BUFFER splited. Records: %d\n", buff_records.size);
}

static void split_buffer_to_records()
{
    if (buff == NULL) {
        printf("ERROR: cant split empty buffer\n");
        return;
    }
    if (buff_records_set) {
        printf("ERROR: BUFFER already splited\n");
        return;
    }

    
    char *fields_separators = command_args[1];
    char *record_separator = command_args[2];
    if ((fields_separators == NULL) || (record_separator == NULL)) {
        printf("Wrong SPLIT usage. Format: s \"[fields separators]\" \"[record separator]\"\n");
        return;
    }

    fields_separators = parse_str_quotas(fields_separators);
    record_separator = parse_str_quotas(record_separator);

    if ((fields_separators == NULL) || (record_separator == NULL)) {
        printf("Wrong SPLIT usage. Format: s \"[fields separators]\" \"[record separator]\"\n");
        return;
    }

    printf("Splited BUFFER:\n\tFields Separators: `%s`\n\tRecord Separator: `%s`\n", fields_separators, record_separator);

    buff_records_clear();
    buff_records = CORE_StrSplitToRecords(buff, fields_separators, record_separator[0]);
    buff_records_set = true;
}

static void print_record(int record_index)
{
    CRecord *r = CRecords_Get(&buff_records, record_index);
    printf("Record %u: [\n", record_index + 1);
    for (uint j = 0; j < r->fields.size; j++) {
        char *f = CRecords_GetField(r, j);
        printf("  %s\n", f);
    }
    printf("]\n\n");
}

static void print_tokens()
{
    if (buff == NULL) {
        printf("ERROR: BUFFER is empty\n");
        return;
    }
    if (!buff_records_set) {
        printf("ERROR: BUFFER is not splited to records\n");
        return;
    }

    const char *record_id = command_args[1];
    
    if (record_id == NULL) {
        for (uint i = 0; i < buff_records.size; i++) {
            print_record(i);
        }
        return;
    }

    int record_id_int = atoi(record_id);
    if (record_id_int > buff_records.size) {
        printf("ERROR: no record with id %d\n", record_id_int);
        return;
    }

    print_record(record_id_int - 1);
    return;
}

int main() 
{
    while (1) {
        printf("> ");
        char command[256];
        fgets(command, sizeof(command), stdin);
        // remove \n
        command[CORE_StrLen(command) - 1] = '\0';
        split_command_to_args(command);
        
        if (command[0] == 'q') {
            break;
        }
        if (command[0] == 'r') {
            process_read();
            continue;
        }
        if (command[0] == 'b') {
            print_buff();
            continue;
        }
        if (command[0] == 's') {
            split_buffer_to_records();
            continue;
        }
        if (command[0] == 't') {
            print_tokens();
            continue;
        }
    }

    buff_records_clear();
    buff_set(NULL, 0);

    return 0;
}

