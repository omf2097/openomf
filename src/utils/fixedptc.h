#ifndef _FIXEDPTC_H_
#define _FIXEDPTC_H_

/*
 * fixedptc.h is a 32-bit or 64-bit fixed point numeric library.
 *
 * 16-bit support has also been added.
 *
 * The symbol FIXEDPT_BITS, if defined before this library header file
 * is included, determines the number of bits in the data type (its "width").
 * The default width is 32-bit (FIXEDPT_BITS=32) and it can be used
 * on any recent C99 compiler. The 64-bit precision (FIXEDPT_BITS=64) is
 * available on compilers which implement 128-bit "long long" types. This
 * precision has been tested on GCC 4.2+.
 *
 * The FIXEDPT_WBITS symbols governs how many bits are dedicated to the
 * "whole" part of the number (to the left of the decimal point). The larger
 * this width is, the larger the numbers which can be stored in the fixedpt
 * number. The rest of the bits (available in the FIXEDPT_FBITS symbol) are
 * dedicated to the fraction part of the number (to the right of the decimal
 * point).
 *
 * Since the number of bits in both cases is relatively low, many complex
 * functions (more complex than div & mul) take a large hit on the precision
 * of the end result because errors in precision accumulate.
 * This loss of precision can be lessened by increasing the number of
 * bits dedicated to the fraction part, but at the loss of range.
 *
 * Adventurous users might utilize this library to build two data types:
 * one which has the range, and one which has the precision, and carefully
 * convert between them (including adding two number of each type to produce
 * a simulated type with a larger range and precision).
 *
 * The ideas and algorithms have been cherry-picked from a large number
 * of previous implementations available on the Internet.
 * Tim Hartrick has contributed cleanup and 64-bit support patches.
 *
 * == Special notes for the 32-bit precision ==
 * Signed 32-bit fixed point numeric library for the 24.8 format.
 * The specific limits are -8388608.999... to 8388607.999... and the
 * most precise number is 0.00390625. In practice, you should not count
 * on working with numbers larger than a million or to the precision
 * of more than 2 decimal places. Make peace with the fact that PI
 * is 3.14 here. :)
 */

/*-
 * Copyright (c) 2010-2012 Ivan Voras <ivoras@freebsd.org>
 * Copyright (c) 2012 Tim Hartrick <tim@edgecast.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef FIXEDPT_BITS
#define FIXEDPT_BITS 32
#endif

#include <stdint.h>

#if FIXEDPT_BITS == 16
typedef int16_t fixedpt;
typedef int32_t fixedptd;
typedef uint16_t fixedptu;
typedef uint32_t fixedptud;
#define FIXEDPT_MIN INT16_MIN
#define FIXEDPT_MAX INT16_MAX
#define FIXEDPTU_MAX UINT16_MAX
#elif FIXEDPT_BITS == 32
typedef int32_t fixedpt;
typedef int64_t fixedptd;
typedef uint32_t fixedptu;
typedef uint64_t fixedptud;
#define FIXEDPT_MIN INT32_MIN
#define FIXEDPT_MAX INT32_MAX
#define FIXEDPTU_MAX UINT32_MAX
#elif FIXEDPT_BITS == 64
typedef int64_t fixedpt;
typedef __int128_t fixedptd;
typedef uint64_t fixedptu;
typedef __uint128_t fixedptud;
#define FIXEDPT_MIN INT64_MIN
#define FIXEDPT_MAX INT64_MAX
#define FIXEDPTU_MAX UINT64_MAX
#else
#error "FIXEDPT_BITS must be equal to 16, 32, or 64"
#endif

#ifndef FIXEDPT_WBITS
#define FIXEDPT_WBITS 24
#endif

#if FIXEDPT_WBITS >= FIXEDPT_BITS
#error "FIXEDPT_WBITS must be less than or equal to FIXEDPT_BITS"
#endif

#define FIXEDPT_VCSID "$Id$"

#define FIXEDPT_FBITS (FIXEDPT_BITS - FIXEDPT_WBITS)
#define FIXEDPT_FMASK (((fixedpt)1 << FIXEDPT_FBITS) - 1)

#define fixedpt_rconst(R) ((fixedpt)((R) * FIXEDPT_ONE + ((R) >= 0 ? 0.5 : -0.5)))
#define fixedpt_fromint(I) ((fixedptd)(I) * FIXEDPT_ONE)
#define fixedpt_toint(F) ((F) / FIXEDPT_ONE)
#define fixedpt_add(A, B) ((A) + (B))
#define fixedpt_sub(A, B) ((A) - (B))
#define fixedpt_xmul(A, B) ((fixedpt)(((fixedptd)(A) * (fixedptd)(B)) / FIXEDPT_ONE))
#define fixedpt_xmul3(A, B, C) fixedpt_xmul(fixedpt_xmul((A), (B)), (C))
#define fixedpt_xdiv(A, B) ((fixedpt)(((fixedptd)(A) * FIXEDPT_ONE) / (fixedptd)(B)))
#define fixedpt_fracpart(A) ((fixedptud)(A) & FIXEDPT_FMASK)

#define FIXEDPT_ONE ((fixedpt)((fixedpt)1 << FIXEDPT_FBITS))
#define FIXEDPT_ONE_HALF (FIXEDPT_ONE >> 1)
#define FIXEDPT_TWO (FIXEDPT_ONE + FIXEDPT_ONE)
#define FIXEDPT_PI fixedpt_rconst(3.14159265358979323846)
#define FIXEDPT_TWO_PI fixedpt_rconst(2 * 3.14159265358979323846)
#define FIXEDPT_HALF_PI fixedpt_rconst(3.14159265358979323846 / 2)
#define FIXEDPT_E fixedpt_rconst(2.7182818284590452354)

/* fixedpt is meant to be usable in environments without floating point support
 * (e.g. microcontrollers, kernels), so we can't use floating point types directly.
 * Putting them only in macros will effectively make them optional. */
#define fixedpt_tofloat(T) ((float)((T) * ((float)(1) / (float)(1L << FIXEDPT_FBITS))))

#if FIXEDPT_MAX <= INT_MAX
#define fixedpt_abs(A) abs((A))
#elif FIXEDPT_MAX <= INT64_MAX
#define fixedpt_abs(A) llabs((A))
#else
#define fixedpt_abs(A) ((A) < 0 ? -(A) : (A))
#endif

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

/* Multiplies two fixedpt numbers, returns the result. */
static inline fixedpt fixedpt_mul(fixedpt A, fixedpt B) {
    return (((fixedptd)A * (fixedptd)B) >> FIXEDPT_FBITS);
}

/* Divides two fixedpt numbers, returns the result. */
static inline fixedpt fixedpt_div(fixedpt A, fixedpt B) {
    return (((fixedptd)A << FIXEDPT_FBITS) / (fixedptd)B);
}

/*
 * Note: adding and substracting fixedpt numbers can be done by using
 * the regular integer operators + and -.
 *
 * Second note: multiplying and dividing a fixedpt number by an integer
 * can be done with the regular integer operators * and /, if you're a
 * bad enough programmer (https://youtu.be/EmZvrEPWx6w?t=18)-- dividing
 * an integer by a fixed point divisor *does* require using fixedpt_xdiv.
 */

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
 * Returns the number of bytes written to `str`.
 */
static inline size_t fixedpt_str(fixedpt A, char *str, size_t bufsize, int max_dec) {
    // boy this function sure is safe with pointers :(
    size_t nint = 0;    // number of integer digits
    size_t nfrac = 0;   // number of fractional digits
    size_t slen = 0;    // string length used
    char tmp[20] = {0}; // can hold a 64 bit integer's 20 digits
    fixedptud fr, ip;
    fixedptud magnitude;
    const fixedptud one = (fixedptud)1 << FIXEDPT_BITS;
    const fixedptud mask = one - 1;
    size_t num_places = 0;

    if(str == NULL || bufsize == 0) {
        return 0;
    }

    if(max_dec == -1) {
#if FIXEDPT_BITS == 16 || FIXEDPT_BITS == 32
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
        // A *= -1;
        magnitude = (fixedptud)(-(A + 1)) + 1;
    } else {
        magnitude = (fixedptud)A;
    }

    ip = fixedpt_toint(magnitude);
    do {
        tmp[nint++] = '0' + ip % 10;
        ip /= 10;
    } while(ip != 0 && nint < sizeof(tmp) - 1);

    while(nint > 0 && slen < bufsize - 1) {
        str[slen++] = tmp[--nint];
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

        while(max_dec == -3 && nfrac > 0 && str[slen - 1] == '0') {
            slen--;
            nfrac--;
        }

        // if we didn't add any decimal values, omit the decimal place
        if(nfrac == 0) {
            slen--;
        }
    }

    int bytes_used = slen < bufsize ? slen : bufsize - 1;
    // always null terminate
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

/* Returns the square root of the given number, or -1 in case of error */
static inline fixedpt fixedpt_sqrt(fixedpt A) {
    int invert = 0;
    int iter = FIXEDPT_FBITS;
    int l, i;

    if(A < 0)
        return (-1);
    if(A == 0 || A == FIXEDPT_ONE)
        return (A);
    if(A < FIXEDPT_ONE && A > 6) {
        invert = 1;
        A = fixedpt_div(FIXEDPT_ONE, A);
    }
    if(A > FIXEDPT_ONE) {
        int s = A;

        iter = 0;
        while(s > 0) {
            s >>= 2;
            iter++;
        }
    }

    /* Newton's iterations */
    l = (A >> 1) + 1;
    for(i = 0; i < iter; i++)
        l = (l + fixedpt_div(A, l)) >> 1;
    if(invert)
        return (fixedpt_div(FIXEDPT_ONE, l));
    return (l);
}

/* Returns the sine of the given fixedpt number.
 * Note: the loss of precision is extraordinary! */
static inline fixedpt fixedpt_sin(fixedpt fp) {
    int sign = 1;
    fixedpt sqr, result;
    const fixedpt SK[2] = {fixedpt_rconst(7.61e-03), fixedpt_rconst(1.6605e-01)};

    fp %= 2 * FIXEDPT_PI;
    if(fp < 0)
        fp = FIXEDPT_PI * 2 + fp;
    if((fp > FIXEDPT_HALF_PI) && (fp <= FIXEDPT_PI))
        fp = FIXEDPT_PI - fp;
    else if((fp > FIXEDPT_PI) && (fp <= (FIXEDPT_PI + FIXEDPT_HALF_PI))) {
        fp = fp - FIXEDPT_PI;
        sign = -1;
    } else if(fp > (FIXEDPT_PI + FIXEDPT_HALF_PI)) {
        fp = (FIXEDPT_PI << 1) - fp;
        sign = -1;
    }
    sqr = fixedpt_mul(fp, fp);
    result = SK[0];
    result = fixedpt_mul(result, sqr);
    result -= SK[1];
    result = fixedpt_mul(result, sqr);
    result += FIXEDPT_ONE;
    result = fixedpt_mul(result, fp);
    return sign * result;
}

/* Returns the cosine of the given fixedpt number */
static inline fixedpt fixedpt_cos(fixedpt A) {
    return (fixedpt_sin(FIXEDPT_HALF_PI - A));
}

/* Returns the tangens of the given fixedpt number */
static inline fixedpt fixedpt_tan(fixedpt A) {
    return fixedpt_div(fixedpt_sin(A), fixedpt_cos(A));
}

/* Returns the value exp(x), i.e. e^x of the given fixedpt number. */
static inline fixedpt fixedpt_exp(fixedpt fp) {
    fixedpt xabs, k, z, R, xp;
    const fixedpt LN2 = fixedpt_rconst(0.69314718055994530942);
    const fixedpt LN2_INV = fixedpt_rconst(1.4426950408889634074);
    const fixedpt EXP_P[5] = {
        fixedpt_rconst(1.66666666666666019037e-01), fixedpt_rconst(-2.77777777770155933842e-03),
        fixedpt_rconst(6.61375632143793436117e-05), fixedpt_rconst(-1.65339022054652515390e-06),
        fixedpt_rconst(4.13813679705723846039e-08),
    };

    if(fp == 0)
        return (FIXEDPT_ONE);
    xabs = fixedpt_abs(fp);
    k = fixedpt_mul(xabs, LN2_INV);
    k += FIXEDPT_ONE_HALF;
    k &= ~FIXEDPT_FMASK;
    if(fp < 0)
        k = -k;
    fp -= fixedpt_mul(k, LN2);
    z = fixedpt_mul(fp, fp);
    /* Taylor */
    R = FIXEDPT_TWO +
        fixedpt_mul(
            z, EXP_P[0] +
                   fixedpt_mul(z, EXP_P[1] +
                                      fixedpt_mul(z, EXP_P[2] + fixedpt_mul(z, EXP_P[3] + fixedpt_mul(z, EXP_P[4])))));
    xp = FIXEDPT_ONE + fixedpt_div(fixedpt_mul(fp, FIXEDPT_TWO), R - fp);
    if(k < 0)
        k = FIXEDPT_ONE >> (-k >> FIXEDPT_FBITS);
    else
        k = FIXEDPT_ONE << (k >> FIXEDPT_FBITS);
    return (fixedpt_mul(k, xp));
}

/* Returns the natural logarithm of the given fixedpt number. */
static inline fixedpt fixedpt_ln(fixedpt x) {
    fixedpt log2, xi;
    fixedpt f, s, z, w, R;
    const fixedpt LN2 = fixedpt_rconst(0.69314718055994530942);
    const fixedpt LG[7] = {fixedpt_rconst(6.666666666666735130e-01), fixedpt_rconst(3.999999999940941908e-01),
                           fixedpt_rconst(2.857142874366239149e-01), fixedpt_rconst(2.222219843214978396e-01),
                           fixedpt_rconst(1.818357216161805012e-01), fixedpt_rconst(1.531383769920937332e-01),
                           fixedpt_rconst(1.479819860511658591e-01)};

    if(x < 0)
        return (0);
    if(x == 0) {
        return FIXEDPT_MAX;
    }

    log2 = 0;
    xi = x;
    while(xi > FIXEDPT_TWO) {
        xi >>= 1;
        log2++;
    }
    f = xi - FIXEDPT_ONE;
    s = fixedpt_div(f, FIXEDPT_TWO + f);
    z = fixedpt_mul(s, s);
    w = fixedpt_mul(z, z);
    R = fixedpt_mul(w, LG[1] + fixedpt_mul(w, LG[3] + fixedpt_mul(w, LG[5]))) +
        fixedpt_mul(z, LG[0] + fixedpt_mul(w, LG[2] + fixedpt_mul(w, LG[4] + fixedpt_mul(w, LG[6]))));
    return (fixedpt_mul(LN2, (log2 << FIXEDPT_FBITS)) + f - fixedpt_mul(s, f - R));
}

/* Returns the logarithm of the given base of the given fixedpt number */
static inline fixedpt fixedpt_log(fixedpt x, fixedpt base) {
    return (fixedpt_div(fixedpt_ln(x), fixedpt_ln(base)));
}

/* Return the power value (n^exp) of the given fixedpt numbers */
static inline fixedpt fixedpt_pow(fixedpt n, fixedpt exp) {
    if(exp == 0)
        return (FIXEDPT_ONE);
    if(n < 0)
        return 0;
    return (fixedpt_exp(fixedpt_mul(fixedpt_ln(n), exp)));
}

#endif
