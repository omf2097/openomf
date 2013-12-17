#include "utils/random.h"

// A simple psuedorandom number generator

static struct random_t rand_state = { 1 };

void random_seed(struct random_t *r, uint32_t seed) {
    r->seed = seed;
}

uint32_t random_get_seed(struct random_t *r) {
    return r->seed;
}

uint32_t random_int(struct random_t *r, uint32_t upperbound) {
    return random_intmax(r) % upperbound;
}

uint32_t random_intmax(struct random_t *r) {
    r->seed = r->seed * 1664525 + 1013904223;
    return r->seed;
}

void rand_seed(uint32_t seed) { random_seed(&rand_state, seed); }
uint32_t rand_get_seed(void) { return random_get_seed(&rand_state); }
uint32_t rand_int(uint32_t upperbound) { return random_int(&rand_state, upperbound); }
uint32_t rand_intmax(void) { return random_intmax(&rand_state);  }

