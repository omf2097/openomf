/** @file main.c
 * @brief Language file parser tool
 * @author Tuomas Virtanen
 * @license MIT
 */

#include "formats/error.h"
#include "formats/language.h"
#include "utils/allocator.h"
#include "utils/cp437.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
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
    exit(EXIT_FAILURE);
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
    long id = strtol(value, &endptr, 10);

    if(*endptr != '\0') {
        error_exit("ID must be a valid integer", *line_number);
    }
    if(language->count != id) {
        char error[100];
        snprintf(error, sizeof error, "Nonsequential ID. Expected %u, got %ld.", language->count, id);
        error_exit(error, *line_number);
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
    char *data_end = data + 8192;
    value = extract_value(line, "Data", *line_number, true);
    char *data_iter = data;
    if(value[0] != '\0') {
        size_t value_len = strlen(value);
        if(data + value_len + 1 > data_end) {
            error_exit("Way too long 'Data:' field", *line_number);
        }
        memcpy(data_iter, value, value_len + 1);
        data_iter += value_len;

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
            value_len = strlen(line);
            if(data_iter + value_len + 1 > data_end) {
                error_exit("Way too long 'Data:' field", *line_number);
            }
            memcpy(data_iter, line, value_len + 1);
            data_iter += value_len;
            *line_number += 1;
        }
    }

    if(desc[0] == '\n') {
        desc[0] = 0;
    }
    if(data_iter > data && data_iter[-1] == '\n')
        // trim final newline
        data_iter[-1] = '\0';
    sd_language_append(language, desc, data);
    free(data);
    free(desc);
    return 1;
}

typedef struct conversion_result {
    cp437_result error_code;
    int string_index;
} conversion_result;

static conversion_result sd_language_to_utf8(sd_language *language) {
    assert(language);
    conversion_result result;
    for(int idx = 0; idx < language->count; idx++) {
        result.string_index = idx;
        char *old_data = language->strings[idx].data;
        size_t sizeof_old_data = strlen(old_data) + 1;
        char old_description[sizeof language->strings[idx].description];
        memcpy(old_description, language->strings[idx].description, sizeof old_description);

        // convert data to utf-8
        size_t sizeof_utf8_data;
        result.error_code = cp437_to_utf8(NULL, 0, &sizeof_utf8_data, old_data, sizeof_old_data);
        if(result.error_code != CP437_SUCCESS)
            return result;
        language->strings[idx].data = omf_malloc(sizeof_utf8_data);
        result.error_code =
            cp437_to_utf8(language->strings[idx].data, sizeof_utf8_data, NULL, old_data, sizeof_old_data);
        if(result.error_code != CP437_SUCCESS) {
            assert(("cp437_to_utf8 should have failed the first time or not the second", 0));
            omf_free(language->strings[idx].data);
            language->strings[idx].data = old_data;
            return result;
        }
        omf_free(old_data);

        // convert description
        // TODO: Use strnlen_s here, check if description is missing its NUL terminator
        size_t sizeof_old_description = strlen(old_description) + 1;
        memset(language->strings[idx].description, '\0', sizeof language->strings[idx].description);
        result.error_code = cp437_to_utf8(language->strings[idx].description, sizeof language->strings[idx].description,
                                          NULL, old_description, sizeof_old_description);
        if(result.error_code != CP437_SUCCESS) {
            memcpy(language->strings[idx].description, old_description, sizeof old_description);
            return result;
        }
    }

    result.error_code = CP437_SUCCESS;
    result.string_index = 0;
    return result;
}

static conversion_result sd_language_from_utf8(sd_language *language) {
    assert(language);
    conversion_result result;
    for(int idx = 0; idx < language->count; idx++) {
        result.string_index = idx;
        char *old_data = language->strings[idx].data;
        size_t sizeof_old_data = strlen(old_data) + 1;
        char old_description[sizeof language->strings[idx].description];
        memcpy(old_description, language->strings[idx].description, sizeof old_description);

        // convert data to DOS CP 437
        size_t sizeof_cp437_data;
        result.error_code = cp437_from_utf8(NULL, 0, &sizeof_cp437_data, old_data, sizeof_old_data);
        if(result.error_code != CP437_SUCCESS)
            return result;
        language->strings[idx].data = omf_malloc(sizeof_cp437_data);
        result.error_code =
            cp437_from_utf8(language->strings[idx].data, sizeof_cp437_data, NULL, old_data, sizeof_old_data);
        if(result.error_code != CP437_SUCCESS) {
            assert(("cp437_from_utf8 should have failed the first time or not the second", 0));
            omf_free(language->strings[idx].data);
            language->strings[idx].data = old_data;
            return result;
        }
        omf_free(old_data);

        // convert description
        // TODO: Use strnlen_s here, check if description is missing its NUL terminator
        size_t sizeof_old_description = strlen(old_description) + 1;
        memset(language->strings[idx].description, '\0', sizeof language->strings[idx].description);
        result.error_code =
            cp437_from_utf8(language->strings[idx].description, sizeof language->strings[idx].description, NULL,
                            old_description, sizeof_old_description);
        if(result.error_code != CP437_SUCCESS) {
            memcpy(language->strings[idx].description, old_description, sizeof old_description);
            return result;
        }
    }

    result.error_code = CP437_SUCCESS;
    result.string_index = 0;
    return result;
}

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file0("f", "file", "<file>", "load OMF language file");
    struct arg_file *input = arg_file0("i", "import", "<file>", "import UTF-8 .TXT file");
    struct arg_int *str = arg_int0("s", "string", "<value>", "display language string number");
    struct arg_file *output = arg_file0("o", "output", "<file>", "compile output language file");
    struct arg_int *check_count =
        arg_int0("c", "check-count", "<NUM>", "Check that language file has this many entries, or bail.");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, vers, file, input, output, str, check_count, end};
    const char *progname = "languagetool";

    bool language_is_utf8 = false;
    sd_language language;
    sd_language_create(&language);
    // assume failure until success happens.
    int main_ret = EXIT_FAILURE;

    // Make sure everything got allocated
    if(arg_nullcheck(argtable) != 0) {
        fprintf(stderr, "%s: insufficient memory\n", progname);
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
        printf("%s v0.2\n", progname);
        printf("Command line One Must Fall 2097 Language file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2013-2024 Tuomas Virtanen & Contributors\n");
        goto exit_0;
    }

    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stderr, end, progname);
        fprintf(stderr, "Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Get strings
    int ret;

    if(file->count > 0) {
        ret = sd_language_load(&language, file->filename[0]);
        language_is_utf8 = false;
        if(ret != SD_SUCCESS) {
            fprintf(stderr, "Language file could not be loaded! Error [%d] %s\n", ret, sd_get_error(ret));
            goto exit_0;
        }
    } else if(input->count > 0) {
        char const *expected_ext = ".TXT";
        if(!input->extension[0] || strcmp(input->extension[0], expected_ext) != 0) {
            fprintf(stderr, "Refusing to open input file %s, does not have expected %s file extension.\n",
                    input->filename[0], expected_ext);
            goto exit_0;
        }
        // parse the supplied text file
        FILE *file = fopen(input->filename[0], "rb");
        if(!file) {
            fprintf(stderr, "Could not open %s\n", input->filename[0]);
            goto exit_0;
        }
        int line = 1;
        language_is_utf8 = true;
        while(read_entry(file, &language, &line)) {
        }
    } else {
        fprintf(stderr, "Please supply -f or -i\n");
        goto exit_0;
    }

    if(check_count->count > 0 && (unsigned)check_count->ival[0] != language.count) {
        fprintf(stderr, "Expected %u entries, got %d!\n", (unsigned)check_count->ival[0], language.count);
        goto exit_0;
    }

    // Print
    const sd_lang_string *ds;
    if(str->count > 0) {
        if(!language_is_utf8) {
            conversion_result result = sd_language_to_utf8(&language);
            if(result.error_code != CP437_SUCCESS) {
                fprintf(stderr, "Error converting to UTF-8! Error %s on language entry %d\n",
                        cp437_result_to_string(result.error_code), result.string_index);
                goto exit_0;
            }
            language_is_utf8 = true;
        }
        unsigned str_id = (unsigned)str->ival[0];
        ds = sd_language_get(&language, str_id);
        if(ds == NULL) {
            fprintf(stderr, "String %d not found!\n", str_id);
            goto exit_0;
        }

        printf("Title: %s\n", ds->description);
        printf("Data: %s\n", ds->data);
    } else if(output->count == 0) {
        if(!language_is_utf8) {
            conversion_result result = sd_language_to_utf8(&language);
            if(result.error_code != CP437_SUCCESS) {
                fprintf(stderr, "Error converting to UTF-8! Error %s on language entry %d\n",
                        cp437_result_to_string(result.error_code), result.string_index);
                goto exit_0;
            }
            language_is_utf8 = true;
        }
        for(unsigned i = 0; i < language.count; i++) {
            ds = sd_language_get(&language, i);
            if(ds != NULL) {
                printf("ID: %d\n", i);
                printf("Title: %s\n", ds->description);
                printf("Data: %s\n", ds->data);
            }
        }
    }

    // Save
    if(output->count > 0) {
        char const *expected_output_extensions[] = {".DAT", ".DAT2", ".LNG", ".LNG2"};
        bool unexpected_extension = true;
        for(int i = 0; i < (sizeof expected_output_extensions) / (sizeof expected_output_extensions[0]); i++) {
            if(output->extension[0] && strcmp(expected_output_extensions[i], output->extension[0]) == 0) {
                unexpected_extension = false;
                break;
            }
        }
        if(unexpected_extension) {
            fprintf(stderr, "Refusing to save language file to %s: unexpected file extension.\n", output->filename[0]);
            goto exit_0;
        }

        if(language_is_utf8) {
            conversion_result result = sd_language_from_utf8(&language);
            if(result.error_code != CP437_SUCCESS) {
                fprintf(stderr, "Error converting from UTF-8! Error %s on language entry %d\n",
                        cp437_result_to_string(result.error_code), result.string_index);
                goto exit_0;
            }
            language_is_utf8 = false;
        }

        ret = sd_language_save(&language, output->filename[0]);
        if(ret != SD_SUCCESS) {
            fprintf(stderr, "Failed saving language file to %s: %s\n", output->filename[0], sd_get_error(ret));
            goto exit_0;
        }
    }

    main_ret = EXIT_SUCCESS;
exit_0:
    sd_language_free(&language);
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return main_ret;
}
