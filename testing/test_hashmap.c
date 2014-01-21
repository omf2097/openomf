#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <utils/hashmap.h>
#include <utils/iterator.h>
#include <stdio.h>

#define TEST_VAL_COUNT 1000

hashmap test_map;
unsigned int test_values[TEST_VAL_COUNT];

void test_hashmap_create(void) {
    hashmap_create(&test_map, 8);
    CU_ASSERT_PTR_NOT_NULL(test_map.buckets);
    CU_ASSERT(hashmap_reserved(&test_map) == 0);
    CU_ASSERT(hashmap_size(&test_map) == 256);
    return;
}

void test_hashmap_free(void) {
    hashmap_free(&test_map);
    CU_ASSERT_PTR_NULL(test_map.buckets);
    CU_ASSERT(test_map.reserved == 0);
    CU_ASSERT(test_map.buckets_x == 0);
    return;
}

void test_hashmap_delete(void) {
    unsigned int *val;
    unsigned int vlen;
    int removed = 0;
    for(unsigned int i = 0; i < TEST_VAL_COUNT; i+=10) {
        CU_ASSERT(hashmap_del(&test_map, &i, sizeof(int)) == 0);
        test_values[i] = 0;
        CU_ASSERT(hashmap_get(&test_map, &i, sizeof(int), (void**)&val, &vlen) == 1);
        CU_ASSERT(val == NULL);
        CU_ASSERT(vlen == 0);
        removed++;
    }
    CU_ASSERT(hashmap_reserved(&test_map) == TEST_VAL_COUNT - removed);
    return;
}

void test_hashmap_insert(void) {
    for(unsigned int i = 0; i < TEST_VAL_COUNT; i++) {
        unsigned int k = TEST_VAL_COUNT - i;
        int *v = hashmap_put(&test_map, &i, sizeof(int), &k, sizeof(int));
        CU_ASSERT_PTR_NOT_NULL(v);
        CU_ASSERT(*v == k);
        test_values[i] = k;
    }
    CU_ASSERT(hashmap_reserved(&test_map) == TEST_VAL_COUNT);
    return;
}

void test_hashmap_get(void) {
    unsigned int *val;
    unsigned int vlen;

    for(unsigned int i = 0; i < TEST_VAL_COUNT; i ++) {
        hashmap_get(&test_map, &i, sizeof(int), (void**)&val, &vlen);
        CU_ASSERT(*val == test_values[i]);
        CU_ASSERT(vlen == sizeof(int));
    }

    return;
}

void test_hashmap_iterator(void) {
    iterator it;
    hashmap_iter_begin(&test_map, &it);
    hashmap_pair *pair;

    unsigned int *val;
    unsigned int *key;

    while((pair = iter_next(&it)) != NULL) {
        key = pair->key;
        val = pair->val;
        CU_ASSERT(pair->keylen == sizeof(int));
        CU_ASSERT(pair->vallen == sizeof(int));
        CU_ASSERT(*key == (TEST_VAL_COUNT - *val));
        CU_ASSERT(test_values[*key] > 0);
        test_values[*key] = 0;
    }
    for(unsigned int i = 0; i < TEST_VAL_COUNT; i++) {
        CU_ASSERT(test_values[i] == 0);
    }
    return;
}

void test_hashmap_iter_del(void) {
    iterator it;
    hashmap_iter_begin(&test_map, &it);
    hashmap_pair *pair;
    while((pair = iter_next(&it)) != NULL) {
        CU_ASSERT(hashmap_delete(&test_map, &it) == 0);
    }
    CU_ASSERT(hashmap_reserved(&test_map) == 0);
}

void test_hashmap_clear(void) {
    // Some test values
    for(unsigned int i = 0; i < TEST_VAL_COUNT; i++) {
        hashmap_put(&test_map, &i, sizeof(int), &i, sizeof(int));
    }

    // Do clear
    hashmap_clear(&test_map);

    // Should be cleared now
    CU_ASSERT(hashmap_reserved(&test_map) == 0);
}

void hashmap_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for hashmap create", test_hashmap_create) == NULL) { return; }
    if(CU_add_test(suite, "Test for hashmap insert operation", test_hashmap_insert) == NULL) { return; }
    if(CU_add_test(suite, "Test for hashmap get operation", test_hashmap_get) == NULL) { return; }
    if(CU_add_test(suite, "Test for hashmap delete operation", test_hashmap_delete) == NULL) { return; }
    if(CU_add_test(suite, "Test for hashmap iterator ", test_hashmap_iterator) == NULL) { return; }
    if(CU_add_test(suite, "Test for hashmap iterator delete operation", test_hashmap_iter_del) == NULL) { return; }
    if(CU_add_test(suite, "Test for hashmap clear operation", test_hashmap_clear) == NULL) { return; }
    if(CU_add_test(suite, "Test for hashmap free operation", test_hashmap_free) == NULL) { return; }
}
