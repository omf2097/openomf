#include "common.h"
#include "platform/time.h"
#include "utils/log.h"

#include <SDL.h>

static void test_time_clock(void) {
    log_init();
    const Uint32 other_before = SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO);
    CU_ASSERT_TRUE_FATAL(time_init());
    CU_ASSERT_TRUE(SDL_WasInit(SDL_INIT_TIMER) & SDL_INIT_TIMER);
    CU_ASSERT_EQUAL(SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO), other_before);

    const uint64_t before = time_ms();
    time_sleep_ms(1);
    const uint64_t after = time_ms();
    CU_ASSERT(after >= before);

    time_close();
    CU_ASSERT_FALSE(SDL_WasInit(SDL_INIT_TIMER) & SDL_INIT_TIMER);
    log_close();
}

void time_test_suite(CU_pSuite suite) {
    ADD_TEST("Monotonic clock and sleep", test_time_clock);
}
