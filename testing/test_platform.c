#include "common.h"
#include "platform/platform.h"

#include <SDL.h>

static void test_platform_clock(void) {
    /* Timer-only initialization avoids opening a video device in headless CI. */
    CU_ASSERT_EQUAL_FATAL(SDL_InitSubSystem(SDL_INIT_TIMER), 0);

    const uint64_t before = platform_time_ms();
    platform_sleep_ms(1);
    const uint64_t after = platform_time_ms();
    CU_ASSERT(after >= before);

    SDL_QuitSubSystem(SDL_INIT_TIMER);
}

void platform_test_suite(CU_pSuite suite) {
    ADD_TEST("Platform monotonic clock and sleep", test_platform_clock);
}
