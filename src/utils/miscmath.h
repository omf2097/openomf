/**
 * @file miscmath.h
 * @brief Miscellaneous mathematical utilities.
 * @details Provides common math operations not available in the standard library,
 *          including clamping, min/max functions, and other numeric utilities.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef MISCMATH_H
#define MISCMATH_H

#include <limits.h>
#include <math.h>
#include <stddef.h>

/** @brief Mathematical constant pi (as a float). */
#define MATH_PI 3.14159265f

/**
 * @brief Clamp an integer to a range.
 * @param val Value to clamp
 * @param min Minimum allowed value
 * @param max Maximum allowed value
 * @return val clamped to [min, max]
 */
static inline int clamp(int val, int min, int max) {
    if(val > max) {
        return max;
    }
    if(val < min) {
        return min;
    }
    return val;
}

/**
 * @brief Clamp a float to a range.
 * @param val Value to clamp
 * @param min Minimum allowed value
 * @param max Maximum allowed value
 * @return val clamped to [min, max]
 */
static inline float clampf(float val, float min, float max) {
    if(val > max) {
        return max;
    }
    if(val < min) {
        return min;
    }
    return val;
}

/**
 * @brief Safely convert a long to an int by clamping.
 * @details Prevents overflow by clamping to INT_MIN/INT_MAX.
 * @param val Long value to convert
 * @return The value as an int, clamped to int range
 */
static inline int clamp_long_to_int(long val) {
    if(val > INT_MAX) {
        return INT_MAX;
    }
    if(val < INT_MIN) {
        return INT_MIN;
    }
    return val;
}

/**
 * @brief Return the maximum of three integers.
 * @param a First value
 * @param b Second value
 * @param c Third value
 * @return The largest of a, b, and c
 */
static inline int max3(int a, int b, int c) {
    int max = a;
    if(b > max) {
        max = b;
    }
    if(c > max) {
        max = c;
    }
    return max;
}

/**
 * @brief Return the maximum of two integers.
 * @param a First value
 * @param b Second value
 * @return The larger of a and b
 */
static inline int max2(int a, int b) {
    return (a > b) ? a : b;
}

/**
 * @brief Return the minimum of two integers.
 * @param a First value
 * @param b Second value
 * @return The smaller of a and b
 */
static inline int min2(int a, int b) {
    return (a > b) ? b : a;
}

/**
 * @brief Return the maximum of two unsigned integers.
 * @param a First value
 * @param b Second value
 * @return The larger of a and b
 */
static inline unsigned umax2(unsigned a, unsigned b) {
    return (a > b) ? a : b;
}

/**
 * @brief Return the minimum of two unsigned integers.
 * @param a First value
 * @param b Second value
 * @return The smaller of a and b
 */
static inline unsigned umin2(unsigned a, unsigned b) {
    return (a > b) ? b : a;
}

/**
 * @brief Calculate the absolute difference between two unsigned integers.
 * @param a First value
 * @param b Second value
 * @return |a - b|
 */
static inline unsigned udist(unsigned a, unsigned b) {
    return (a > b) ? a - b : b - a;
}

/**
 * @brief Calculate signed distance from a to b.
 * @param a First value
 * @param b Second value
 * @return Signed distance (positive if a < b, negative if a > b)
 */
static inline float dist(float a, float b) {
    return fabsf((a < b ? a : b) - (a > b ? a : b)) * (a < b ? 1.0f : -1.0f);
}

/**
 * @brief Return the minimum of two size_t values.
 * @param a First value
 * @param b Second value
 * @return The smaller of a and b
 */
static inline size_t smin2(size_t a, size_t b) {
    return (a > b) ? b : a;
}

/**
 * @brief Calculate x raised to the power y (unsigned integers).
 * @param x Base
 * @param y Exponent
 * @return x^y
 */
static inline unsigned powu(unsigned x, unsigned y) {
    unsigned ret = 1;
    while(y--) {
        ret *= x;
    }
    return ret;
}

#endif // MISCMATH_H
