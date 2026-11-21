/**
 * @file vec.h
 * @brief 2D vector mathematics.
 * @details Provides 2D vector types and operations for integer,
 *          floating-point, and fixed point coordinates. Used throughout
 *          the engine for positions, velocities, and other 2D quantities.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef VEC_H
#define VEC_H

#include <math.h>
#include "utils/fixedpt.h"

/**
 * @brief 2D vector with float components.
 */
typedef struct vec2fp {
    float x; ///< X component
    float y; ///< Y component
} vec2fp;

/**
 * @brief 2D vector with integer components.
 */
typedef struct vec2i {
    int x; ///< X component
    int y; ///< Y component
} vec2i;

/**
 * @brief 2D vector with fixedpt components.
 */
typedef struct vec2pt {
    fixedpt x; ///< X component
    fixedpt y; ///< Y component
} vec2pt;

/**
 * @brief Add two integer vectors.
 * @param a First vector
 * @param b Second vector
 * @return Sum of a and b
 */
static inline vec2i vec2i_add(vec2i a, vec2i b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

/**
 * @brief Subtract two integer vectors.
 * @param a First vector
 * @param b Second vector (to subtract)
 * @return a minus b
 */
static inline vec2i vec2i_sub(vec2i a, vec2i b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

/**
 * @brief Component-wise multiply two integer vectors.
 * @param a First vector
 * @param b Second vector
 * @return Component-wise product
 */
static inline vec2i vec2i_mult(vec2i a, vec2i b) {
    a.x *= b.x;
    a.y *= b.y;
    return a;
}

/**
 * @brief Add two float vectors.
 * @param a First vector
 * @param b Second vector
 * @return Sum of a and b
 */
static inline vec2fp vec2f_add(vec2fp a, vec2fp b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

/**
 * @brief Subtract two float vectors.
 * @param a First vector
 * @param b Second vector (to subtract)
 * @return a minus b
 */
static inline vec2fp vec2f_sub(vec2fp a, vec2fp b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

/**
 * @brief Component-wise multiply two float vectors.
 * @param a First vector
 * @param b Second vector
 * @return Component-wise product
 */
static inline vec2fp vec2f_mult(vec2fp a, vec2fp b) {
    a.x *= b.x;
    a.y *= b.y;
    return a;
}

/**
 * @brief Add two fixed point vectors.
 * @param a First vector
 * @param b Second vector
 * @return Sum of a and b
 */
static inline vec2pt vec2pt_add(vec2pt a, vec2pt b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

/**
 * @brief Subtract two fixed point vectors.
 * @param a First vector
 * @param b Second vector (to subtract)
 * @return a minus b
 */
static inline vec2pt vec2pt_sub(vec2pt a, vec2pt b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

/**
 * @brief Component-wise multiply two fixed point vectors.
 * @param a First vector
 * @param b Second vector
 * @return Component-wise product
 */
static inline vec2pt vec2pt_mult(vec2pt a, vec2pt b) {
    a.x = fixedpt_mul(a.x, b.x);
    a.y = fixedpt_mul(a.y, b.y);
    return a;
}

/**
 * @brief Convert a float vector to an integer vector.
 * @details Components are truncated toward zero.
 * @param f Float vector to convert
 * @return Integer vector with truncated components
 */
static inline vec2i vec2f_to_i(vec2fp f) {
    vec2i i;
    i.x = f.x;
    i.y = f.y;
    return i;
}

/**
 * @brief Convert an integer vector to a float vector.
 * @param i Integer vector to convert
 * @return Float vector with the same component values
 */
static inline vec2fp vec2i_to_f(vec2i i) {
    vec2fp f;
    f.x = i.x;
    f.y = i.y;
    return f;
}

/**
 * @brief Convert a fixed point vector to an integer vector.
 * @details Components are truncated toward zero.
 * @param pt Fixed point vector to convert
 * @return Integer vector with truncated components
 */
static inline vec2i vec2pt_to_i(vec2pt pt) {
    vec2i i;
    i.x = fixedpt_toint(pt.x);
    i.y = fixedpt_toint(pt.y);
    return i;
}

/**
 * @brief Calculate the magnitude (length) of a float vector.
 * @param a Vector to measure
 * @return Length of the vector
 */
static inline float vec2f_mag(vec2fp a) {
    return sqrtf(a.x * a.x + a.y * a.y);
}

/**
 * @brief Normalize a float vector to unit length.
 * @param a Vector to normalize
 * @return Unit vector in the same direction
 */
static inline vec2fp vec2f_norm(vec2fp a) {
    float mag = vec2f_mag(a);
    a.x /= mag;
    a.y /= mag;
    return a;
}

/**
 * @brief Calculate the distance between two float vectors.
 * @param a First point
 * @param b Second point
 * @return Euclidean distance between a and b
 */
static inline float vec2f_dist(vec2fp a, vec2fp b) {
    return vec2f_mag(vec2f_sub(a, b));
}

/**
 * @brief Calculate the magnitude (length) of a fixed point vector.
 * @param a Vector to measure
 * @return Length of the vector
 */
static inline float vec2pt_mag(vec2pt a) {
    return fixedpt_sqrt(fixedpt_mul(a.x, a.x) + fixedpt_mul(a.y, a.y));
}

/**
 * @brief Normalize a fixed point vector to unit length.
 * @param a Vector to normalize
 * @return Unit vector in the same direction
 */
static inline vec2pt vec2pt_norm(vec2pt a) {
    fixedpt mag = vec2pt_mag(a);
    a.x = fixedpt_div(a.x, mag);
    a.y = fixedpt_div(a.y, mag);
    return a;
}

/**
 * @brief Calculate the distance between two fixed point vectors.
 * @param a First point
 * @param b Second point
 * @return Euclidean distance between a and b
 */
static inline float vec2pt_dist(vec2pt a, vec2pt b) {
    return vec2pt_mag(vec2pt_sub(a, b));
}

/**
 * @brief Create an integer vector from components.
 * @param x X component
 * @param y Y component
 * @return New integer vector
 */
static inline vec2i vec2i_create(int x, int y) {
    vec2i v;
    v.x = x;
    v.y = y;
    return v;
}

/**
 * @brief Create a float vector from components.
 * @param x X component
 * @param y Y component
 * @return New float vector
 */
static inline vec2fp vec2fp_create(float x, float y) {
    vec2fp v;
    v.x = x;
    v.y = y;
    return v;
}

/**
 * @brief Create a fixed point vector from components.
 * @param x X component
 * @param y Y component
 * @return New fixed point vector
 */
static inline vec2pt vec2pt_create(fixedpt x, fixedpt y) {
    vec2pt v;
    v.x = x;
    v.y = y;
    return v;
}

#endif // VEC_H
