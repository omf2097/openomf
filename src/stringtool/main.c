/** @file main.c
  * @brief Animation string database insert tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <argtable2.h>
#include <sqlite3.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include <string.h>

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

const char *mbasename(const char *filename) {
    const char *slash = strrchr(filename, '/');
    if(!slash) return filename;
    return slash + 1;
}

const char *create_str = " \
    CREATE TABLE files ( \
        id INTEGER PRIMARY KEY, \
        name VARCHAR(32), \
        type VARCHAR(2) \
    ); \
    CREATE TABLE strings ( \
        id INTEGER PRIMARY KEY, \
        str TEXT, \
        anim_id INTEGER, \
        type VARCHAR(32), \
        file INTEGER, \
        FOREIGN KEY(file) REFERENCES files(id) \
    ); \
";

typedef struct {
    char type[4];
    int (*cb)(sqlite3 *db, const char* path, const char* name);
} cbs;

int add_file_info(sqlite3 *db, long *file_id, const char *name, const char *ext) {
    // Add file
    sqlite3_stmt *query;
    int ret;
    ret = sqlite3_prepare(db, "INSERT INTO files (name,type) VALUES (?,?)", -1, &query, NULL);
    if(ret != SQLITE_OK) {
        goto add_file_err_0;
    }
    sqlite3_bind_text(query, 1, name, -1, NULL);
    sqlite3_bind_text(query, 2, ext, -1, NULL);
    ret = sqlite3_step(query);
    if(ret != SQLITE_DONE) {
        goto add_file_err_0;
    }
    *file_id = sqlite3_last_insert_rowid(db);

    sqlite3_finalize(query);
    return 0;

add_file_err_0:
    printf("Error: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(query);
    return 1;
}

int add_string(sqlite3 *db, long file_id, const char *string, int anim_id, const char* type) {
    // Add file
    sqlite3_stmt *query;
    int ret;
    ret = sqlite3_prepare(db, "INSERT INTO strings (str,file,anim_id,type) VALUES (?,?,?,?)", -1, &query, NULL);
    if(ret != SQLITE_OK) {
        goto add_str_err_0;
    }
    sqlite3_bind_text(query, 1, string, -1, NULL);
    sqlite3_bind_int64(query, 2, file_id);
    sqlite3_bind_int(query, 3, anim_id);
    sqlite3_bind_text(query, 4, type, -1, NULL);
    ret = sqlite3_step(query);
    if(ret != SQLITE_DONE) {
        goto add_str_err_0;
    }
    sqlite3_finalize(query);
    return 0;

add_str_err_0:
    printf("Error: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(query);
    return 1;
}

int read_af(sqlite3 *db, const char* path, const char* name) {
    // Load AF file
    int ret;
    sd_af_file af;
    sd_af_create(&af);
    ret = sd_af_load(&af, path);
    if(ret) {
        printf(" * Unable to load AF file! Make sure the file exists and is a valid AF file.\n");
        goto af_err_0;
    }
    
    // Some vars
    long file_id;

    // Add file information
    printf(" * Writing File data ... ");
    if(add_file_info(db, &file_id, name, "AF")) {
        goto af_err_0;
    }
    printf("OK!\n");

    printf(" * Writing animation data ... ");
    for(int m = 0; m < 70; m++) {
        if(af.moves[m]) {
            sd_move *afm = af.moves[m];
            sd_animation *ani = afm->animation;

            add_string(db, file_id, afm->footer_string, m, "Move Footer");
            add_string(db, file_id, ani->anim_string, m, "Animation String");

            // Extra strings
            if(ani->extra_string_count > 0) {
                for(int e = 0; e < ani->extra_string_count; e++) {
                    add_string(db, file_id, ani->extra_strings[e], m, "Extra String");
                }
            }

        }
    }
    printf("OK!\n");

    sd_af_free(&af);
    return 0;

af_err_0:
    sd_af_free(&af);
    return 1;
}

int read_bk(sqlite3 *db, const char* path, const char* name) {
    // Load AF file
    int ret;
    sd_bk_file bk;
    sd_bk_create(&bk);
    ret = sd_bk_load(&bk, path);
    if(ret) {
        printf(" * Unable to load BK file! Make sure the file exists and is a valid BK file.\n");
        goto bk_err_0;
    }
    
    // Some vars
    long file_id;

    // Add file information
    printf(" * Writing file data ... ");
    if(add_file_info(db, &file_id, name, "BK")) {
        goto bk_err_0;
    }
    printf("OK!\n");

    printf(" * Writing animation data ... ");
    for(int m = 0; m < 50; m++) {
        if(bk.anims[m]) {
            sd_bk_anim *bka = bk.anims[m];
            sd_animation *ani = bka->animation;
            add_string(db, file_id, ani->anim_string, m, "Animation String");
            if(ani->extra_string_count > 0) {
                for(int e = 0; e < ani->extra_string_count; e++) {
                    add_string(db, file_id, ani->extra_strings[e], m, "Extra String");
                }
            }
        }
    }
    printf("OK!\n");

    sd_bk_free(&bk);
    return 0;

bk_err_0:
    sd_bk_free(&bk);
    return 1;
}

const cbs types[] = {
    {"AF\0", read_af},
    {"BK\0", read_bk}
};

#define COUNT_TYPES 2

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *infile = arg_file0("i", "input", "<file>", "Input AF/BK file");
    struct arg_file *outfile = arg_file0("o", "output", "<file>", "Output Sqlite database.");
    struct arg_lit *format = arg_lit0("f", "format", "Creates an empty database to the output file.");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,infile,outfile,format,end};
    const char* progname = "stringtool";
    
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
        arg_print_glossary(stdout, argtable, "%-30s %s\n");
        goto exit_0;
    }
    
    // Handle version
    if(vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 Anim. string database inserter.\n");
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
    
    if(outfile->count == 0 || (infile->count == 0 && format->count == 0)) {
        printf("Either --input and --output or --format and --output flags have to be set.\n");
        goto exit_0;
    }

    // Set some variables
    int ret;
    sqlite3 *db;
    const char *filepath = infile->filename[0];
    const char *filename = mbasename(filepath);
    const char *ext = get_filename_ext(filename);

    printf("Parsing file '%s' with type '%s'.\n", filename, ext);

    // Open output db
    ret = sqlite3_open(outfile->filename[0], &db);
    if(ret) {
        printf(" * Unable to open database: %s.\n", sqlite3_errmsg(db));
        goto exit_0;
    }

    // Create database file
    if(format->count > 0) {
        printf(" * Preformatting database file ... ");

        char *err = NULL;
        ret = sqlite3_exec(db, create_str, NULL, NULL, &err);
        if(ret != SQLITE_OK) {
            printf("Error: %s\n", err);
            sqlite3_free(err);
            goto exit_1;
        }

        printf("OK\n");
    }

    // Handle 
    for(int i = 0; i < COUNT_TYPES; i++) {
        cbs c = types[i];
        if(strcmp(c.type, ext) == 0) {
            c.cb(db, filepath, filename);
            break;
        }
    }

exit_1:
    sqlite3_close(db);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
