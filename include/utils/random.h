#ifndef _RANDOM_H
#define _RANDOM_H

#include <stdint.h>

/* Note: do not typedef random here because it will clash with stdlib's (linux?) random() */

struct random_t {
    uint32_t seed;
};

void random_seed(struct random_t *r, uint32_t seed);
uint32_t random_get_seed(struct random_t *r);

/* Return a random integer in 0 <= r < upperbound */
uint32_t random_int(struct random_t *r, uint32_t upperbound);

/* Return a random integer in 0 <= r <= UINT_MAX */
uint32_t random_intmax(struct random_t *r);

/* Return a random float in 0 <= r <= 1.0f */
float random_float(struct random_t *r);


/* Same as the above but keeps an internal state
 * Use as a replacement for rand()
*/
void rand_seed(uint32_t seed);
uint32_t rand_get_seed(void);
uint32_t rand_int(uint32_t upperbound);
uint32_t rand_intmax(void);
float rand_float(void);

#endif // _RANDOM_H
