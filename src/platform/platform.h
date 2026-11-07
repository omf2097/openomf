#ifndef OMF_PLATFORM_H
#define OMF_PLATFORM_H

#include <stdbool.h>
#include <stdint.h>

/* Initialize/shut down the platform facilities used by the game. */
bool platform_init(void);
void platform_close(void);

/* Monotonic elapsed milliseconds and a best-effort delay. */
uint64_t platform_time_ms(void);
void platform_sleep_ms(uint32_t milliseconds);

#endif // OMF_PLATFORM_H
