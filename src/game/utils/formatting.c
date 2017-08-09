#include <stdio.h>
#include "game/utils/formatting.h"

void score_format(unsigned int score, char *buf, int maxlen) {
    unsigned int n = 0;
    unsigned int scale = 1;
    while(score >= 1000) {
        n = n + scale * (score % 1000);
        score /= 1000;
        scale *= 1000;
    }
    int len = snprintf(buf, maxlen, "%u", score);
    while(scale != 1 && len < maxlen) {
        scale /= 1000;
        score = n / scale;
        n = n  % scale;
        len += snprintf(buf + len, maxlen - len, ",%03u", score);
    }
}
