#include "utils/smallbuffer.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdbool.h>

static char const random_data[400] =
    "Voluptatem qui illo aut incidunt ut explicabo porro. Eum magni quam aut fugiat. Et cum sint nobis rerum saepe.\n"
    "Blanditiis sint et quo. Voluptas harum molestias molestiae et cupiditate sed.Ratione eos et magnam omnis id.\n"
    "Ullam cupiditate illum dolor voluptatibus odit. Nam omnis itaque suscipit illum sit quaerat"
    "ipsum. Sed eveniet repudiandae voluptates inventore quidem at similique.\n";
static char const oops_all_nul_bytes[400] = "";

void test_smallbuffer(void) {
    smallbuffer sb;
    smallbuffer_create(&sb);
    CU_ASSERT_EQUAL(smallbuffer_capacity(&sb), 0);

    smallbuffer_resize(&sb, 4);
    CU_ASSERT_EQUAL(smallbuffer_capacity(&sb), 4);
    memcpy(smallbuffer_data(&sb), random_data, 4);
    smallbuffer_resize(&sb, 60);
    CU_ASSERT_EQUAL(smallbuffer_capacity(&sb), 60);
    CU_ASSERT(memcmp(smallbuffer_data(&sb), random_data, 4) == 0);
    CU_ASSERT(memcmp(smallbuffer_data(&sb) + 4, oops_all_nul_bytes, 60 - 4) == 0);

    // try to trigger an asan violation if the allocation didn't happen correctly.
    memcpy(smallbuffer_data(&sb), random_data, 60);

    smallbuffer_free(&sb);
}

typedef struct largebuffer {
    smallbuffer sb;
    char extra_cap[32];
} largebuffer;

void test_largebuffer(void) {
    largebuffer lb;
    smallbuffer_create(&lb.sb);

    smallbuffer_resize_with_custom_selfsize(&lb.sb, 32, sizeof(lb));
    CU_ASSERT_EQUAL(smallbuffer_capacity(&lb.sb), 32);
    memcpy(smallbuffer_data(&lb.sb), random_data, 32);
    smallbuffer_resize_with_custom_selfsize(&lb.sb, 400, sizeof(lb));
    CU_ASSERT_EQUAL(smallbuffer_capacity(&lb.sb), 400);
    CU_ASSERT(memcmp(smallbuffer_data(&lb.sb), random_data, 32) == 0);
    CU_ASSERT(memcmp(smallbuffer_data(&lb.sb) + 32, oops_all_nul_bytes, 400 - 32) == 0);

    // try to trigger an asan violation if the allocation didn't happen correctly.
    memcpy(smallbuffer_data(&lb.sb), random_data, 400);

    smallbuffer_free(&lb.sb);
}

void smallbuffer_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for smallbuffer", test_smallbuffer) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for smallbuffer with extra inline capacity", test_largebuffer) == NULL) {
        return;
    }
}
