#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <utils/sprite_packer.h>

void test_sprite_packer_create(void) {
    sprite_packer *packer = sprite_packer_create(256, 512);
    CU_ASSERT_PTR_NOT_NULL(packer);

    CU_ASSERT(sprite_packer_get_width(packer) == 256);
    CU_ASSERT(sprite_packer_get_height(packer) == 512);

    sprite_packer_free(&packer);
    CU_ASSERT_PTR_NULL(packer);
}

void test_sprite_packer_alloc_single(void) {
    sprite_packer *packer = sprite_packer_create(256, 256);
    sprite_region region;

    CU_ASSERT_TRUE(sprite_packer_alloc(packer, 64, 32, &region));
    CU_ASSERT(region.x == 0);
    CU_ASSERT(region.y == 0);
    CU_ASSERT(region.w == 64);
    CU_ASSERT(region.h == 32);

    sprite_packer_free(&packer);
}

void test_sprite_packer_alloc_multiple(void) {
    sprite_packer *packer = sprite_packer_create(256, 256);
    sprite_region regions[4];

    // Allocate four 64x64 rectangles
    for(int i = 0; i < 4; i++) {
        CU_ASSERT_TRUE(sprite_packer_alloc(packer, 64, 64, &regions[i]));
        CU_ASSERT(regions[i].w == 64);
        CU_ASSERT(regions[i].h == 64);
    }

    // Verify no overlaps
    for(int a = 0; a < 4; a++) {
        for(int b = a + 1; b < 4; b++) {
            const bool a_left_of_b = regions[a].x + regions[a].w <= regions[b].x;
            const bool b_left_of_a = regions[b].x + regions[b].w <= regions[a].x;
            const bool a_above_b = regions[a].y + regions[a].h <= regions[b].y;
            const bool b_above_a = regions[b].y + regions[b].h <= regions[a].y;
            CU_ASSERT_TRUE(a_left_of_b || b_left_of_a || a_above_b || b_above_a);
        }
    }

    sprite_packer_free(&packer);
}

void test_sprite_packer_alloc_exact_fit(void) {
    sprite_packer *packer = sprite_packer_create(128, 128);
    sprite_region region;

    // Fill entire area with one rectangle
    CU_ASSERT_TRUE(sprite_packer_alloc(packer, 128, 128, &region));
    CU_ASSERT(region.x == 0);
    CU_ASSERT(region.y == 0);
    CU_ASSERT(region.w == 128);
    CU_ASSERT(region.h == 128);

    // No more space should be available
    CU_ASSERT_FALSE(sprite_packer_alloc(packer, 1, 1, &region));

    sprite_packer_free(&packer);
}

void test_sprite_packer_alloc_too_large(void) {
    sprite_packer *packer = sprite_packer_create(64, 64);
    sprite_region region;

    CU_ASSERT_FALSE(sprite_packer_alloc(packer, 128, 32, &region));  // Try something too wide
    CU_ASSERT_FALSE(sprite_packer_alloc(packer, 32, 128, &region));  // Try something too tall
    CU_ASSERT_FALSE(sprite_packer_alloc(packer, 128, 128, &region)); // Try something too tall AND wide
    CU_ASSERT_TRUE(sprite_packer_alloc(packer, 32, 32, &region));    // This should fit

    sprite_packer_free(&packer);
}

void test_sprite_packer_reset(void) {
    sprite_packer *packer = sprite_packer_create(128, 128);
    sprite_region region;

    // Fill entire area and verify
    CU_ASSERT_TRUE(sprite_packer_alloc(packer, 128, 128, &region));
    CU_ASSERT_FALSE(sprite_packer_alloc(packer, 1, 1, &region));

    sprite_packer_reset(packer); // Act

    // Should be able to allocate again
    CU_ASSERT_TRUE(sprite_packer_alloc(packer, 128, 128, &region));
    CU_ASSERT(region.x == 0);
    CU_ASSERT(region.y == 0);

    sprite_packer_free(&packer);
}

void sprite_packer_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "Test sprite_packer create", test_sprite_packer_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test sprite_packer alloc single", test_sprite_packer_alloc_single) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test sprite_packer alloc multiple", test_sprite_packer_alloc_multiple) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test sprite_packer alloc exact fit", test_sprite_packer_alloc_exact_fit) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test sprite_packer alloc too large", test_sprite_packer_alloc_too_large) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test sprite_packer reset", test_sprite_packer_reset) == NULL) {
        return;
    }
}
