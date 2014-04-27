#include <stdio.h>
#include "game/utils/formatting.h"

void score_format(unsigned int score, char *buf) {
    unsigned int n = 0;
    unsigned int scale = 1;
    while(score >= 1000) {
        n = n + scale * (score % 1000);
        score /= 1000;
        scale *= 1000;
    }
    int len = sprintf(buf, "%u", score);
    while(scale != 1) {
        scale /= 1000;
        score = n / scale;
        n = n  % scale;
        len += sprintf(buf + len, ",%03u", score);
    }
}
