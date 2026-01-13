#include <CUnit/CUnit.h>
#include <utils/hashmap.h>
#include <utils/iterator.h>

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
    hashmap_put_str(&test_map, "test_key", "test_a", 6);
    CU_ASSERT(hashmap_reserved(&test_map) == 1);

    hashmap_del_str(&test_map, "test_key");

    unsigned int *val;
    unsigned int len;
    CU_ASSERT(hashmap_get_str(&test_map, "test_key", (void **)&val, &len) == 1);
    CU_ASSERT(val == NULL);
    CU_ASSERT(len == 0);
    CU_ASSERT(hashmap_reserved(&test_map) == 0);

    hashmap_free(&test_map);
    return;
}

void test_hashmap_iterator(void) {
    hashmap test_map;
    hashmap_create(&test_map);
    hashmap_put_str(&test_map, "test_a", "test_a", 6);
    hashmap_put_str(&test_map, "test_b", "test_b", 6);

    iterator it;
    hashmap_iter_begin(&test_map, &it);
    hashmap_pair *pair;

    foreach(it, pair) {
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
    hashmap_put_str(&test_map, "test_a", "test_a", 6);

    iterator it;
    hashmap_iter_begin(&test_map, &it);
    hashmap_pair *pair;
    foreach(it, pair) {
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
    for(unsigned int i = 0; i < 1000; i++) {
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

    hashmap_put_int(&test_map, 1, &c_value, sizeof(int));
    CU_ASSERT_EQUAL(test_map.capacity, 4);
    c_value--;

    hashmap_put_int(&test_map, 2, &c_value, sizeof(int));
    CU_ASSERT_EQUAL(test_map.capacity, 4);
    c_value--;

    hashmap_put_int(&test_map, 3, &c_value, sizeof(int));
    CU_ASSERT_EQUAL(test_map.capacity, 4);
    c_value--;

    hashmap_put_int(&test_map, 4, &c_value, sizeof(int));
    CU_ASSERT_EQUAL(test_map.capacity, 8);

    hashmap_free(&test_map);
}

void test_hashmap_update_value(void) {
    hashmap test_map;
    hashmap_create(&test_map);

    unsigned int val1 = 100;
    hashmap_put_int(&test_map, 42, &val1, sizeof(unsigned int));
    CU_ASSERT_EQUAL(hashmap_reserved(&test_map), 1);

    // Overwrite with a new value
    unsigned int val2 = 200;
    hashmap_put_int(&test_map, 42, &val2, sizeof(unsigned int));
    CU_ASSERT_EQUAL(hashmap_reserved(&test_map), 1);

    // verify
    unsigned int *result;
    CU_ASSERT_EQUAL(hashmap_get_int(&test_map, 42, (void **)&result, NULL), 0);
    CU_ASSERT_EQUAL(*result, 200);

    hashmap_free(&test_map);
}

void test_hashmap_resize_integrity(void) {
    hashmap test_map;
    hashmap_create(&test_map);

    // Insert enough stuff to trigger multiple resizes (4, 8, 16, ...)
    for(unsigned int i = 0; i < 100; i++) {
        unsigned int value = i * 10;
        hashmap_put_int(&test_map, i, &value, sizeof(unsigned int));
    }

    CU_ASSERT_EQUAL(hashmap_reserved(&test_map), 100);
    CU_ASSERT(test_map.capacity >= 100);

    // Verify all entries are still there with correct values
    for(unsigned int i = 0; i < 100; i++) {
        unsigned int *result;
        CU_ASSERT_EQUAL(hashmap_get_int(&test_map, i, (void **)&result, NULL), 0);
        CU_ASSERT_EQUAL(*result, i * 10);
    }

    hashmap_free(&test_map);
}

// Just make sure everythign works together
void test_hashmap_stuff(void) {
    hashmap test_map;
    hashmap_create(&test_map);

    for(unsigned int i = 0; i < 500; i++) {
        unsigned int value = i;
        hashmap_put_int(&test_map, i, &value, sizeof(unsigned int));
    }
    CU_ASSERT_EQUAL(hashmap_reserved(&test_map), 500);

    // Delete every other entry
    for(unsigned int i = 0; i < 500; i += 2) {
        hashmap_del_int(&test_map, i);
    }
    CU_ASSERT_EQUAL(hashmap_reserved(&test_map), 250);

    // Verify
    for(unsigned int i = 0; i < 500; i++) {
        unsigned int *result;
        const int ret = hashmap_get_int(&test_map, i, (void **)&result, NULL);
        if(i % 2 == 0) {
            CU_ASSERT_EQUAL(ret, 1);
        } else {
            CU_ASSERT_EQUAL(ret, 0);
            CU_ASSERT_EQUAL(*result, i);
        }
    }

    // Overwrite existing
    for(unsigned int i = 1; i < 500; i += 2) {
        unsigned int value = i + 1000;
        hashmap_put_int(&test_map, i, &value, sizeof(unsigned int));
    }
    CU_ASSERT_EQUAL(hashmap_reserved(&test_map), 250);

    // Verify
    for(unsigned int i = 1; i < 500; i += 2) {
        unsigned int *result;
        CU_ASSERT_EQUAL(hashmap_get_int(&test_map, i, (void **)&result, NULL), 0);
        CU_ASSERT_EQUAL(*result, i + 1000);
    }

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
    if(CU_add_test(suite, "Test for hashmap value update", test_hashmap_update_value) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for hashmap resize integrity", test_hashmap_resize_integrity) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for hashmap with lots of ops", test_hashmap_stuff) == NULL) {
        return;
    }
}
