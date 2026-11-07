#include "utils/fixedpt.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <integer.decimal>\n", argv[0]);
        exit(1);
    }
    const char *error_r;
    fixedpt val;
    if(fixedpt_from_c(argv[1], &val, &error_r) < 0) {
        fprintf(stderr, "Invalid decimal number %s: %s\n", argv[1], error_r);
        exit(1);
    }
    printf("str: %s\n", fixedpt_cstr(val, -3));
    printf("hex: 0x%x\n", (unsigned)val);
    printf("int: %d\n", val);

    return 0;
}
