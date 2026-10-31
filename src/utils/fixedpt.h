/**
 * @file fixedpt.h
 * @brief Fixed point arithmetic for game simulation.
 * @details Wraps the vendored fixedptc library (src/vendored/fixedptc).
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef FIXEDPT_H
#define FIXEDPT_H

#include <stddef.h>
#include <stdint.h>

#define FIXEDPT_BITS 32
#define FIXEDPT_WBITS 24

// Drop the upstream string functions as unsafe.
#define fixedpt_str fixedptc_upstream_str
#define fixedpt_cstr fixedptc_upstream_cstr
#include "vendored/fixedptc/fixedptc.h"
#undef fixedpt_str
#undef fixedpt_cstr

#if FIXEDPT_BITS == 32
#define FIXEDPT_MIN INT32_MIN
#define FIXEDPT_MAX INT32_MAX
#define FIXEDPTU_MAX UINT32_MAX
#define FIXEDPT_WCHARS 10 // 32 bit numbers can use at most 10 decimal digits
#elif FIXEDPT_BITS == 64
#define FIXEDPT_MIN INT64_MIN
#define FIXEDPT_MAX INT64_MAX
#define FIXEDPTU_MAX UINT64_MAX // 64 bit numbers can use at most 20 decimal digits
#define FIXEDPT_WCHARS 20
#else
#error "FIXEDPT_BITS must be equal to 32 or 64"
#endif

#define fixedpt_xmul3(A, B, C) fixedpt_xmul(fixedpt_xmul((A), (B)), (C))

/*
 * Note: adding and substracting fixedpt numbers can be done by using
 * the regular integer operators + and -.
 *
 * Second note: multiplying and dividing a fixedpt number by an integer
 * can be done with the regular integer operators * and /, if you're a
 * bad enough programmer (https://youtu.be/EmZvrEPWx6w?t=18)-- dividing
 * an integer by a fixed point divisor *does* require using fixedpt_xdiv.
 */

static inline fixedpt fixedpt_clamp(fixedpt val, fixedpt min, fixedpt max) {
    if(val > max)
        return max;
    if(val < min)
        return min;
    return val;
}

static inline fixedpt fixedpt_max2(fixedpt a, fixedpt b) {
    return a > b ? a : b;
}

static inline fixedpt fixedpt_min2(fixedpt a, fixedpt b) {
    return a < b ? a : b;
}

// TODO: calculate FIXEDPT_STR_BUFSIZE? or just keep it large enough for fixedpt_str to not blow up.
#define FIXEDPT_STR_BUFSIZE 32

/**
 * Convert the given fixedpt number to a decimal string.
 * The max_dec argument specifies how many decimal digits to the right
 * of the decimal point to generate. If set to -1, the "default" number
 * of decimal digits will be used (2 for 32-bit fixedpt width, 10 for
 * 64-bit fixedpt width); If set to -2, "all" of the digits will
 * be returned, meaning there will be invalid, bogus digits outside the
 * specified precisions. If set to -3, all the zero digits will be trimmed
 * from the end of the decimal digits.
 *
 * Returns the number of bytes written to `str`, not counting the null terminator.
 */
size_t fixedpt_str(fixedpt A, char *str, size_t bufsize, int max_dec);

/* Converts the given fixedpt number into a string, using a static
 * (non-threadsafe) string buffer */
static inline char *fixedpt_cstr(const fixedpt A, const int max_dec) {
    static char str[25];

    fixedpt_str(A, str, sizeof(str), max_dec);
    return (str);
}

#endif // FIXEDPT_H
