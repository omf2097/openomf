#ifndef OMF_TIME_H
#define OMF_TIME_H

#include <stdbool.h>
#include <stdint.h>

bool time_init(void);
void time_close(void);

/* Monotonic elapsed milliseconds and a best-effort delay. */
uint64_t time_ms(void);
void time_sleep_ms(uint32_t milliseconds);

#endif // OMF_TIME_H
