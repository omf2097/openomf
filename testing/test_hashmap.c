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

void test_hashmap_put_get_int(void) {
    hashmap test;
    hashmap_create(&test);
    int int_value1 = 1024;
    hashmap_put_int(&test, 100, &int_value1, sizeof(int));
    hashmap_put_int(&test, 100, &int_value1, sizeof(int));
    int in_value2 = 2048;
    hashmap_put_int(&test, 101, &in_value2, sizeof(int));
    int in_value3 = 4096;
    hashmap_put_int(&test, 102, &in_value3, sizeof(int));

    CU_ASSERT(hashmap_reserved(&test) == 3);
    iterator it;
    hashmap_iter_begin(&test, &it);
    hashmap_pair *pair;
    foreach(it, pair) {
        int key = *(int *)pair->key;
        int value = *(int *)pair->value;
        if(key == 100) {
            CU_ASSERT(value == 1024);
        } else if(key == 101) {
            CU_ASSERT(value == 2048);
        } else if(key == 102) {
            CU_ASSERT(value == 4096);
            hashmap_delete(&test, &it);
        } else {
            CU_ASSERT(false);
        }
    }
    CU_ASSERT(hashmap_reserved(&test) == 2);

    int *value2;
    unsigned int value_len;
    hashmap_get_int(&test, 100, (void **)&value2, &value_len);
    CU_ASSERT(*value2 == 1024);
    CU_ASSERT(value_len == sizeof(int));

    hashmap_get_int(&test, 101, (void **)&value2, &value_len);
    CU_ASSERT(*value2 == 2048);
    CU_ASSERT(value_len == sizeof(int));

    int ret = hashmap_get_int(&test, 103, (void **)&value2, &value_len);
    CU_ASSERT(ret == 1);
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
    if(CU_add_test(suite, "Test for hashmap put/get/iterate int", test_hashmap_put_get_int) == NULL) {
        return;
    }
}
