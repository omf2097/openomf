#include "game/utils/formatting.h"
#include <stdio.h>

void score_format(int signed_score, char *buf, int maxlen) {
    unsigned int n = 0;
    unsigned int scale = 1;
    unsigned int score = (unsigned int)(signed_score);
    int len = 0;
    if(signed_score < 0) {
        len = snprintf(buf, maxlen, "-");
        score = (unsigned int)(-1 * signed_score);
    }
    while(score >= 1000) {
        n = n + scale * (score % 1000);
        score /= 1000;
        scale *= 1000;
    }
    len += snprintf(buf + len, maxlen, "%u", score);
    while(scale != 1 && len < maxlen) {
        scale /= 1000;
        score = n / scale;
        n = n % scale;
        len += snprintf(buf + len, maxlen - len, ",%03u", score);
    }
}
