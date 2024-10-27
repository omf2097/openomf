/** @file main.c
 * @brief Language file parser tool
 * @author Tuomas Virtanen
 * @license MIT
 */

#include "formats/error.h"
#include "formats/language.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if ARGTABLE2_FOUND
#include <argtable2.h>
#elif ARGTABLE3_FOUND
#include <argtable3.h>
#endif

#define MAX_LINE 2048
#define MAX_DATA 8192 // Data field cannot exceed 32 bytes
#define MAX_TITLE 32

// Function to trim whitespace from both ends of a string
void trim(char *str) {
    char *end;
    while(isspace((unsigned char)*str))
        str++;
    if(*str == 0)
        return;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
}

void error_exit(const char *message, int line_number) {
    fprintf(stderr, "Error on line %d: %s\n", line_number, message);
    abort();
}

// Function to extract value after colon with validation
char *extract_value(const char *line, const char *field_name, int line_number, bool allow_empty) {
    char *colon = strchr(line, ':');
    if(!colon) {
        char error[100];
        snprintf(error, sizeof(error), "Missing colon in %s field", field_name);
        error_exit(error, line_number);
    }

    char *value = colon + 2;

    if(!allow_empty && strlen(value) == 0) {
        char error[100];
        snprintf(error, sizeof(error), "Empty %s field", field_name);
        error_exit(error, line_number);
    }

    return value;
}

int read_entry(FILE *file, sd_language *language, int *line_number) {
    char line[MAX_LINE];
    if(!fgets(line, sizeof(line), file)) {
        // EOF is ok here
        return 0;
    }
    if(strncmp(line, "ID:", 3) != 0) {
        error_exit("Expected 'ID:' field", *line_number);
    }

    char *value = extract_value(line, "ID", *line_number, false);
    trim(value);
    char *endptr;
    strtol(value, &endptr, 10);

    if(*endptr != '\0') {
        error_exit("ID must be a valid integer", *line_number);
    }
    *line_number += 1;

    if(!fgets(line, sizeof(line), file)) {
        error_exit("Unexpected EOF while reading Title", *line_number);
    }

    trim(line);

    if(strncmp(line, "Title:", 6) != 0) {
        error_exit("Expected 'Title:' field", *line_number);
    }

    char *title = extract_value(line, "Title", *line_number, true);
    char *desc = strdup(title);
    trim(desc);

    *line_number += 1;
    // Read Data header
    if(!fgets(line, sizeof(line), file)) {
        error_exit("Unexpected EOF while reading Data", *line_number);
    }

    if(strncmp(line, "Data:", 5) != 0) {
        error_exit("Expected 'Data:' field", *line_number);
    }

    char *data = malloc(8192);
    memset(data, 0, 8192);
    value = extract_value(line, "Data", *line_number, true);
    if(strlen(value) > 0) {
        strncpy(data, value, strlen(value));
        data[strlen(value) + 1] = 0;

        *line_number += 1;
        // Read data body until next entry or EOF
        while(fgets(line, sizeof(line), file)) {
            // Check if this is the start of a new entry
            if(strncmp(line, "ID:", 3) == 0) {
                // Rewind to start of this line
                fseek(file, -strlen(line), SEEK_CUR);
                break;
            }

            // Append to existing data
            strncat(data, line, strlen(line));
            *line_number += 1;
        }
    }

    if(desc[0] == '\n') {
        desc[0] = 0;
    }
    data[strlen(data) - 1] = 0;
    sd_language_append(language, desc, data);
    free(data);
    free(desc);
    return 1;
}

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file0("f", "file", "<file>", "language file");
    struct arg_file *input = arg_file0("i", "input", "<file>", "exported language file to re-import");
    struct arg_int *str = arg_int0("s", "string", "<value>", "Select language string number");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output compiled language file");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, vers, file, input, output, str, end};
    const char *progname = "languagetool";

    // Make sure everything got allocated
    if(arg_nullcheck(argtable) != 0) {
        printf("%s: insufficient memory\n", progname);
        goto exit_0;
    }

    // Parse arguments
    int nerrors = arg_parse(argc, argv, argtable);

    // Handle help
    if(help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("\nArguments:\n");
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }

    // Handle version
    if(vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 Language file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2013 Tuomas Virtanen\n");
        goto exit_0;
    }

    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Get strings
    sd_language language;
    sd_language_create(&language);
    int ret;

    if(file->count > 0) {
        ret = sd_language_load(&language, file->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Language file could not be loaded! Error [%d] %s\n", ret, sd_get_error(ret));
            goto exit_1;
        }
    } else if(input->count > 0) {
        // parse the supplied text file
        FILE *file = fopen(input->filename[0], "r");
        if(!file) {
            fprintf(stderr, "Could not open %s", input->filename[0]);
            goto exit_1;
        }
        int line = 1;
        while(read_entry(file, &language, &line)) {
        }
    }

    // Print
    const sd_lang_string *ds;
    if(str->count > 0) {
        unsigned str_id = (unsigned)str->ival[0];
        ds = sd_language_get(&language, str_id);
        if(ds == NULL) {
            printf("String %d not found!\n", str_id);
            goto exit_1;
        }

        printf("Title: %s\n", ds->description);
        printf("Data: %s\n", ds->data);
    } else {
        for(unsigned i = 0; i < language.count; i++) {
            ds = sd_language_get(&language, i);
            if(ds != NULL) {
                printf("ID: %d\n", i);
                printf("Title: %s\n", ds->description);
                printf("Data: %s\n", ds->data);
            }
        }
    }
    else {
        fprintf(stderr, "Please supply -f or -i\n");
        goto exit_1;
    }

    // Saving
    if(output->count > 0) {
        ret = sd_language_save(&language, output->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Failed saving language file to %s: %s", output->filename[0], sd_get_error(ret));
        }
    }

exit_1:
    sd_language_free(&language);
exit_0:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}
