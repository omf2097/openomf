/**
 * @file random.h
 * @brief Pseudo-random number generation.
 * @details Provides both instanced and global random number generators.
 */

#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

/* Note: do not typedef random here because it will clash with stdlib's (linux?) random() */

/**
 * @brief Random number generator state.
 * @details Holds the seed for reproducible random sequences.
 */
struct random_t {
    uint32_t seed; ///< Current RNG state/seed
};

/**
 * @brief Initialize RNG with a seed value.
 * @param r RNG state to initialize
 * @param seed Seed value for the generator
 */
void random_seed(struct random_t *r, uint32_t seed);

/**
 * @brief Get the current seed value.
 * @param r RNG state to query
 * @return Current seed value
 */
uint32_t random_get_seed(struct random_t *r);

/**
 * @brief Generate a random integer in range [0, upperbound).
 * @param r RNG state to use
 * @param upperbound Upper bound (exclusive)
 * @return Random integer in [0, upperbound-1]
 */
uint32_t random_int(struct random_t *r, uint32_t upperbound);

/**
 * @brief Generate a random integer in range [0, UINT32_MAX].
 * @param r RNG state to use
 * @return Random integer covering the full 32-bit range
 */
uint32_t random_intmax(struct random_t *r);

/**
 * @brief Generate a random float in range [0.0, 1.0].
 * @param r RNG state to use
 * @return Random float in [0.0, 1.0]
 */
float random_float(struct random_t *r);

/**
 * @brief Initialize the global RNG with a seed value.
 * @details Use as a replacement for srand().
 * @param seed Seed value for the generator
 */
void rand_seed(uint32_t seed);

/**
 * @brief Get the current global RNG seed.
 * @return Current seed value
 */
uint32_t rand_get_seed(void);

/**
 * @brief Generate a random integer in range [0, upperbound).
 * @details Uses the global RNG state.
 * @param upperbound Upper bound (exclusive)
 * @return Random integer in [0, upperbound-1]
 */
uint32_t rand_int(uint32_t upperbound);

/**
 * @brief Generate a random integer in range [0, UINT32_MAX].
 * @details Uses the global RNG state.
 * @return Random integer covering the full 32-bit range
 */
uint32_t rand_intmax(void);

/**
 * @brief Generate a random float in range [0.0, 1.0].
 * @details Uses the global RNG state.
 * @return Random float in [0.0, 1.0]
 */
float rand_float(void);

#endif // RANDOM_H
