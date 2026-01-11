/**
 * @file vec.h
 * @brief 2D vector mathematics.
 * @details Provides 2D vector types and operations for both integer and
 *          floating-point coordinates. Used throughout the engine for
 *          positions, velocities, and other 2D quantities.
 */

#ifndef VEC_H
#define VEC_H

#include <math.h>

/**
 * @brief 2D vector with float components.
 */
typedef struct vec2f {
    float x; ///< X component
    float y; ///< Y component
} vec2f;

/**
 * @brief 2D vector with integer components.
 */
typedef struct vec2i {
    int x; ///< X component
    int y; ///< Y component
} vec2i;

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
static inline vec2f vec2f_add(vec2f a, vec2f b) {
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
static inline vec2f vec2f_sub(vec2f a, vec2f b) {
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
static inline vec2f vec2f_mult(vec2f a, vec2f b) {
    a.x *= b.x;
    a.y *= b.y;
    return a;
}

/**
 * @brief Convert a float vector to an integer vector.
 * @details Components are truncated toward zero.
 * @param f Float vector to convert
 * @return Integer vector with truncated components
 */
static inline vec2i vec2f_to_i(vec2f f) {
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
static inline vec2f vec2i_to_f(vec2i i) {
    vec2f f;
    f.x = i.x;
    f.y = i.y;
    return f;
}

/**
 * @brief Calculate the magnitude (length) of a float vector.
 * @param a Vector to measure
 * @return Length of the vector
 */
static inline float vec2f_mag(vec2f a) {
    return sqrtf(a.x * a.x + a.y * a.y);
}

/**
 * @brief Normalize a float vector to unit length.
 * @param a Vector to normalize
 * @return Unit vector in the same direction
 */
static inline vec2f vec2f_norm(vec2f a) {
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
static inline float vec2f_dist(vec2f a, vec2f b) {
    return vec2f_mag(vec2f_sub(a, b));
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
static inline vec2f vec2f_create(float x, float y) {
    vec2f v;
    v.x = x;
    v.y = y;
    return v;
}

#endif // VEC_H
