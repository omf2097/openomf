#include "utils/sstream.h"

static bool is_digit(const char c) {
    return c >= '0' && c <= '9';
}

static int read_sign(sstream *s) {
    const char c = sstream_peek(s);
    if(c == '-') {
        sstream_skip(s, 1);
        return -1;
    }
    if(c == '+') {
        sstream_skip(s, 1);
        return 1;
    }
    return 1;
}

static bool read_digits(sstream *s, long *magnitude) {
    long acc = 0;
    bool found = false;
    char c = sstream_peek(s);
    while(is_digit(c)) {
        acc = acc * 10 + (c - '0');
        sstream_skip(s, 1);
        found = true;
        c = sstream_peek(s);
    }
    *magnitude = acc;
    return found;
}

static long clamp_long(const long value, const long min, const long max) {
    if(value < min) {
        return min;
    }
    if(value > max) {
        return max;
    }
    return value;
}

bool sstream_read_long(sstream *s, long *out, const long min, const long max) {
    const int sign = read_sign(s);
    long magnitude;
    if(!read_digits(s, &magnitude)) {
        *out = 0;
        return false; // no digits, though there might have been a sign.
    }
    *out = clamp_long(sign * magnitude, min, max);
    return true;
}
