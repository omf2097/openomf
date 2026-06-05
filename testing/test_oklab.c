#include "utils/allocator.h"
#include <CUnit/CUnit.h>
#include <vendored/oklab/oklab.h>
#include <float.h>

void test_rgb_to_oklab(void) {
    oklab_color c;
    // white
    c = rgb_to_oklab(255, 255, 255);

    CU_ASSERT(c.L - 1.0 < FLT_EPSILON);
    CU_ASSERT(c.a - 0.0 < FLT_EPSILON);
    CU_ASSERT(c.b - 0.0 < FLT_EPSILON);

    // magenta
    c = rgb_to_oklab(255, 0, 255);

    CU_ASSERT(c.L - 0.702 < DBL_EPSILON);
    CU_ASSERT(c.a - 0.275 < DBL_EPSILON);
    CU_ASSERT(c.b - -0.169 < DBL_EPSILON);

    // yellow
    c = rgb_to_oklab(255, 255, 0);

    CU_ASSERT(c.L - 0.968 < DBL_EPSILON);
    CU_ASSERT(c.a - 0.071 < DBL_EPSILON);
    CU_ASSERT(c.b - 0.199 < DBL_EPSILON);

    // red
    c = rgb_to_oklab(255, 0, 0);

    CU_ASSERT(c.L - 0.628 < DBL_EPSILON);
    CU_ASSERT(c.a - 0.225 < DBL_EPSILON);
    CU_ASSERT(c.b - 0.126 < DBL_EPSILON);

    // random color, a nice blue
    c = rgb_to_oklab(48, 26, 150);

    CU_ASSERT(c.L - 0.350 < DBL_EPSILON);
    CU_ASSERT(c.a - 0.029 < DBL_EPSILON);
    CU_ASSERT(c.b - -0.182 < DBL_EPSILON);

    // random color, a leaf green
    c = rgb_to_oklab(74, 171, 33);

    CU_ASSERT(c.L - 0.658 < DBL_EPSILON);
    CU_ASSERT(c.a - -0.143 < DBL_EPSILON);
    CU_ASSERT(c.b - 0.127 < DBL_EPSILON);
}

void test_oklab_to_rgb(void) {
    oklab_color c;

    // random color, a leaf green
    c.L = 0.658;
    c.a = -0.143;
    c.b = 0.127;

    uint8_t r, g, b;

    oklab_to_rgb(c, &r, &g, &b);

    // some innaccuricies here because of using 3 decimal places for oklab colors
    CU_ASSERT_EQUAL(r, 75);
    CU_ASSERT_EQUAL(g, 171);
    CU_ASSERT_EQUAL(b, 32);
}

void oklab_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "Test RGB to OKLab", test_rgb_to_oklab) == NULL) {
        return;
    }

    if(CU_add_test(suite, "Test OKLab to RGB", test_oklab_to_rgb) == NULL) {
        return;
    }
}
