#include "video/renderers/common.h"
#include <CUnit/CUnit.h>

#define ASSERT_RECT(a, b)                                                                                              \
    CU_ASSERT(a.w == b.w);                                                                                             \
    CU_ASSERT(a.h == b.h);                                                                                             \
    CU_ASSERT(a.x == b.x);                                                                                             \
    CU_ASSERT(a.y == b.y)

void test_find_resolution_for_aspect_ratio(void) {
    SDL_Rect vga = {0, 0, 640, 480};    // 4:3
    SDL_Rect sxga = {0, 0, 1280, 1024}; // 5:4
    SDL_Rect fhd = {0, 0, 1920, 1080};  // 16:9
    SDL_Rect dst, test;

    // Should match VGA exactly, since vga is 4:2
    find_resolution_for_aspect_ratio(&dst, &vga, 4, 3);
    test = (SDL_Rect){0, 0, 640, 480};
    ASSERT_RECT(dst, test);

    // SXGA should add bars on top and bottom
    find_resolution_for_aspect_ratio(&dst, &sxga, 4, 3);
    test = (SDL_Rect){0, 32, 1280, 960};
    ASSERT_RECT(dst, test);

    // Full HD should add bars to left and right
    find_resolution_for_aspect_ratio(&dst, &fhd, 4, 3);
    test = (SDL_Rect){240, 0, 1440, 1080};
    ASSERT_RECT(dst, test);
}

void video_common_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "Test for find_resolution_for_aspect_ratio", test_find_resolution_for_aspect_ratio) == NULL) {
        return;
    }
}
