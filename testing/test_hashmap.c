#include <CUnit/CUnit.h>
#include <utils/hashmap.h>
#include <utils/iterator.h>

#define TEST_VAL_COUNT 1000

void test_hashmap_create(void) {
    hashmap test_map;
    hashmap_create(&test_map);
    CU_ASSERT_PTR_NOT_NULL(test_map.buckets);
    CU_ASSERT(hashmap_reserved(&test_map) == 0);
    CU_ASSERT(hashmap_size(&test_map) == 4);
    hashmap_free(&test_map);
    CU_ASSERT_PTR_NULL(test_map.buckets);
    CU_ASSERT(test_map.reserved == 0);
    CU_ASSERT(test_map.capacity == 0);
    return;
}

void test_hashmap_delete(void) {
    hashmap test_map;
    hashmap_create(&test_map);
    hashmap_sput(&test_map, "test_key", "test_a", 6);
    CU_ASSERT(hashmap_reserved(&test_map) == 1);

    hashmap_sdel(&test_map, "test_key");

    unsigned int *val;
    unsigned int len;
    CU_ASSERT(hashmap_sget(&test_map, "test_key", (void **)&val, &len) == 1);
    CU_ASSERT(val == NULL);
    CU_ASSERT(len == 0);
    CU_ASSERT(hashmap_reserved(&test_map) == 0);

    hashmap_free(&test_map);
    return;
}

void test_hashmap_iterator(void) {
    hashmap test_map;
    hashmap_create(&test_map);
    hashmap_sput(&test_map, "test_a", "test_a", 6);
    hashmap_sput(&test_map, "test_b", "test_b", 6);

    iterator it;
    hashmap_iter_begin(&test_map, &it);
    hashmap_pair *pair;

    while((pair = iter_next(&it)) != NULL) {
        CU_ASSERT_EQUAL(pair->key_len, 7);
        CU_ASSERT_EQUAL(pair->value_len, 6);
        CU_ASSERT(memcmp(pair->key, "test_a", 6) == 0 || memcmp(pair->key, "test_b", 6) == 0);
        CU_ASSERT(memcmp(pair->key, pair->value, 6) == 0);
    }

    hashmap_free(&test_map);
    return;
}

void test_hashmap_iter_del(void) {
    hashmap test_map;
    hashmap_create(&test_map);
    hashmap_sput(&test_map, "test_a", "test_a", 6);

    iterator it;
    hashmap_iter_begin(&test_map, &it);
    hashmap_pair *pair;
    while((pair = iter_next(&it)) != NULL) {
        CU_ASSERT(memcmp(pair->key, "test_a", 6) == 0);
        CU_ASSERT(memcmp(pair->value, "test_a", 6) == 0);
        CU_ASSERT(hashmap_delete(&test_map, &it) == 0);
    }
    CU_ASSERT(hashmap_reserved(&test_map) == 0);

    hashmap_free(&test_map);
}

void test_hashmap_clear(void) {
    hashmap test_map;
    hashmap_create(&test_map);

    // Some test values
    for(unsigned int i = 0; i < TEST_VAL_COUNT; i++) {
        hashmap_put(&test_map, &i, sizeof(int), &i, sizeof(int));
    }

    // Do clear
    hashmap_clear(&test_map);

    // Should be cleared now
    CU_ASSERT(hashmap_reserved(&test_map) == 0);

    hashmap_free(&test_map);
}

void hashmap_test_autoresize(void) {
    hashmap test_map;
    hashmap_create(&test_map);
    CU_ASSERT_EQUAL(test_map.capacity, 4);

    unsigned int c_value = 0xFFFF;

    hashmap_iput(&test_map, 1, &c_value, sizeof(int));
    CU_ASSERT_EQUAL(test_map.capacity, 4);
    c_value--;

    hashmap_iput(&test_map, 2, &c_value, sizeof(int));
    CU_ASSERT_EQUAL(test_map.capacity, 4);
    c_value--;

    hashmap_iput(&test_map, 3, &c_value, sizeof(int));
    CU_ASSERT_EQUAL(test_map.capacity, 4);
    c_value--;

    hashmap_iput(&test_map, 4, &c_value, sizeof(int));
    CU_ASSERT_EQUAL(test_map.capacity, 8);

    hashmap_free(&test_map);
}

void hashmap_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for hashmap create", test_hashmap_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for hashmap delete operation", test_hashmap_delete) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for hashmap iterator ", test_hashmap_iterator) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for hashmap iterator delete operation", test_hashmap_iter_del) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for hashmap clear operation", test_hashmap_clear) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for hashmap auto resize", hashmap_test_autoresize) == NULL) {
        return;
    }
}
