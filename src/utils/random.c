#include <limits.h>
#include "utils/random.h"

// A simple psuedorandom number generator

static random rand = { 1 };

void random_seed(random *r, uint32_t seed) {
    r->seed = seed;
}

uint32_t random_int(random *r, uint32_t upperbound) {
    // TODO the output is slightly biased, but good enough for now
    return random_intmax(r) % upperbound;
}

uint32_t random_intmax(random *r) {
    r->seed = r->seed * 1664525 + 1013904223;
    return r->seed;
}

void rand_seed(uint32_t seed) { random_seed(&rand, seed); }
uint32_t rand_get_seed(void) { return rand.seed; }
uint32_t rand_int(uint32_t upperbound) { return random_int(&rand, upperbound); }
uint32_t rand_intmax(void) { return random_intmax(&rand);  }

