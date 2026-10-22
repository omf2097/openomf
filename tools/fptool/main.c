#include "utils/fixedpt.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

//+#define FIXEDPT_DEF3(I, D) (((fixedpt)(I) << FIXEDPT_FBITS) + (((fixedpt)(D) << FIXEDPT_FBITS) / 1000))

static int create_fixedpoint(const char *arg, const char **error_r) {
    char *end;

    errno = 0;
    long integer_part = strtol(arg, &end, 10);

    if(arg == end) {
        *error_r = "Argument does not start with integer";
        return -1;
    } else if(errno == ERANGE) {
        *error_r = "The integer part overflowed the 'long' type limits.";
        return -1;
    } else if(*end != '.') {
        return 0;
    }

    const char *decimal_start = end + 1;
    long decimal_part = strtol(decimal_start, &end, 10);

    if(decimal_start == end) {
        return 0;
    }

    printf("Integer part: %ld\n", integer_part);
    printf("Decimal part: %ld\n", decimal_part);

    return 0;
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Usage: %s <integer[.decimal]>\n", argv[0]);
        exit(1);
    }
    const char *error_r;
    if(create_fixedpoint(argv[1], &error_r) < 0) {
        printf("Invalid parameters for fixed point: %s\n", error_r);
        exit(1);
    }

    return 0;
}
