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
#elif FIXEDPT_BITS == 64
#define FIXEDPT_MIN INT64_MIN
#define FIXEDPT_MAX INT64_MAX
#define FIXEDPTU_MAX UINT64_MAX
#else
#error "FIXEDPT_BITS must be equal to 32 or 64"
#endif

#define fixedpt_xmul3(A, B, C) fixedpt_xmul(fixedpt_xmul((A), (B)), (C))

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

#endif // FIXEDPT_H
