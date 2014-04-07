/** @file main.c
  * @brief Animation string parser tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int read_next_int(const char *str, int *pos) {
    int opos = 0;
    char buf[20];
    memset(buf, 0, 20);
    if (str[*pos] == '-' && str[(*pos)+1] >= '0' && str[(*pos)+1] <= '9') {
        buf[opos] = str[*pos];
        (*pos)++;
        opos++;
    }
    if(str[*pos] == '+') {
        (*pos)++;
    }
    while(str[*pos] >= '0' && str[*pos] <= '9') {
        buf[opos] = str[*pos];
        (*pos)++;
        opos++;
    }

    if(opos == 0) return 0;
    return atoi(buf);
}

int main(int argc, char* argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_str *astr = arg_str1("s", "str", "<str>", "Animation string");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,astr,end};
    const char* progname = "omf_parse";
    
    // Make sure everything got allocated
    if(arg_nullcheck(argtable) != 0) {
        printf("%s: insufficient memory\n", progname);
        goto exit_0;
    }
    
    // Parse arguments
    int nerrors = arg_parse(argc, argv, argtable);
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }
    
    // Handle version
    if(vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 Animation string parser\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2014 Tuomas Virtanen\n");
        goto exit_0;
    }
    
    // Handle help
    if(help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("\nArguments:\n");
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }

    // Print some data
    printf("Parsing \"%s\".\n\n", *astr->sval);

    // Walk through the string
    const char *str = *astr->sval;
    char test[4];
    int len = strlen(str);
    int i = 0;

    char otemp[2048];
    otemp[0] = 0;
    int frame_number = 1;
    while(i < len) {
        if(str[i] >= 'A' && str[i] <= 'Z') {
            char frame = str[i];
            i++;
            int flen = read_next_int(str, &i);
            int ikey = (int)(frame-65);
            printf("%d. Frame %d: '%c%d'\n", frame_number, ikey, frame, flen);
            frame_number++;
            printf(otemp);
            printf("\n");
            otemp[0] = 0;
            i++;
            continue;
        }
        if(str[i] >= 'a' && str[i] <= 'z') {
            int found = 0;
            for(int k = 3; k > 0; k--) {
                memcpy(test, str+i, k);
                test[k] = 0;

                int req_param;
                const char *desc = NULL;
                if(tag_info(test, &req_param, &desc) == 0) {
                    if(desc == NULL) {
                        desc = "Unknown";
                    }

                    i += k;
                    found = 1;
                    if(req_param) {
                        int param = read_next_int(str, &i);
                        char tmp[5];
                        sprintf(tmp, "%d", param);
                        sprintf(otemp+strlen(otemp)," * %-4s %-4s %s\n", test, tmp, desc);
                    } else {
                        sprintf(otemp+strlen(otemp)," * %-4s      %s\n", test, desc);
                    }
                    k = 0;
                }
            }
            if(!found) {
                sprintf(otemp+strlen(otemp)," * %-9c <Unlisted tag!>\n", str[i]);
                i++;
            }
        }

    }

exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
