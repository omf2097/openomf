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

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

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

#undef fixedpt_fromint
#define fixedpt_fromint(I) ((fixedptd)(I) * FIXEDPT_ONE)
#undef fixedpt_toint
#define fixedpt_toint(F) ((F) / FIXEDPT_ONE)
#undef fixedpt_xmul
#define fixedpt_xmul(A, B) ((fixedpt)(((fixedptd)(A) * (fixedptd)(B)) / FIXEDPT_ONE))
#undef fixedpt_xdiv
#define fixedpt_xdiv(A, B) ((fixedpt)(((fixedptd)(A) * FIXEDPT_ONE) / (fixedptd)(B)))
#define fixedpt_xmul3(A, B, C) fixedpt_xmul(fixedpt_xmul((A), (B)), (C))

#undef fixedpt_fracpart
#define fixedpt_fracpart(A) ((fixedptud)(A) & FIXEDPT_FMASK)

#undef fixedpt_abs
#if FIXEDPT_MAX <= INT_MAX
#define fixedpt_abs(A) abs((A))
#elif FIXEDPT_MAX <= INT64_MAX
#define fixedpt_abs(A) llabs((A))
#else
#define fixedpt_abs(A) ((A) < 0 ? -(A) : (A))
#endif

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
static inline size_t fixedpt_str(fixedpt A, char *str, size_t bufsize, int max_dec) {
    // boy this function sure is safe with pointers :(
    size_t nint = 0;  // number of integer digits
    size_t nfrac = 0; // number of fractional digits
    size_t slen = 0;  // string length used

    // accumulator for integer digits, can hold the max number of digits for FIXEDPT_BITS
    char acc[FIXEDPT_WCHARS] = {0};
    fixedptud fr, ip;
    fixedptud magnitude = (fixedptud)(fixedptd)A;
    const fixedptud one = (fixedptud)1 << FIXEDPT_BITS;
    const fixedptud mask = one - 1;
    size_t num_places = 0;

    // return if the destination is null or too small
    if(str == NULL || bufsize <= 0) {
        return 0;
    } else if(bufsize < 2) {
        str[0] = '\0';
        return 0;
    }

    if(max_dec == -1) {
#if FIXEDPT_BITS == 32
#if FIXEDPT_FBITS < 16
        num_places = 2;
#else
        num_places = 4;
#endif
#elif FIXEDPT_BITS == 64
        num_places = 10;
#else
#error Invalid width
#endif
    } else if(max_dec == -2) {
        num_places = 15;
    } else if(max_dec == -3) {
        num_places = 15;
    } else if(max_dec < 0) {
        // other negative values of max_dec not allowed
        return 0;
    } else {
        num_places = max_dec;
    }

    if(A < 0) {
        str[slen++] = '-';
        magnitude *= -1;
    }

    // Accumulate the integer digits in reverse order
    // by dividing the integer component by 10 each time.
    // Use do/while so at least a 0 is written
    ip = fixedpt_toint(magnitude);
    do {
        acc[nint++] = '0' + ip % 10;
        ip /= 10;
    } while(ip != 0 && nint < sizeof(acc) - 1);

    // reverse the accumulated digits into the output string
    while(nint > 0 && slen < bufsize - 1) {
        str[slen++] = acc[--nint];
    }

    // check if we want, and have space for, decimal digits
    if(num_places > 0 && slen < bufsize - 1) {
        str[slen++] = '.';

        fr = (fixedpt_fracpart(magnitude) << FIXEDPT_WBITS) & mask;
        while(nfrac < num_places && slen < bufsize - 1) {
            fr = (fr & mask) * 10;

            str[slen++] = '0' + (fr >> FIXEDPT_BITS) % 10;
            nfrac++;
        }

        // trim any trailing 0s, if desired
        while(max_dec == -3 && nfrac > 0 && str[slen - 1] == '0') {
            slen--;
            nfrac--;
        }

        // if we didn't add any decimal values, omit the decimal place
        if(nfrac == 0) {
            slen--;
        }
    }

    size_t bytes_used = slen < bufsize ? slen : bufsize - 1;
    str[bytes_used] = '\0';
    return bytes_used;
}

/* Converts the given fixedpt number into a string, using a static
 * (non-threadsafe) string buffer */
static inline char *fixedpt_cstr(const fixedpt A, const int max_dec) {
    static char str[25];

    fixedpt_str(A, str, sizeof(str), max_dec);
    return (str);
}

#endif // FIXEDPT_H
