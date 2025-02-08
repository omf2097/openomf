#include "utils/allocator.h"
#include <CUnit/CUnit.h>
#include <utils/array.h>
#include <utils/iterator.h>

static const char *test_data1 = "TEST1";
static const char *test_data2 = "TEST2";

void test_array_create(void) {
    array test_array;
    array_create(&test_array);
    CU_ASSERT(test_array.filled == 0);
    CU_ASSERT(test_array.allocated_size != 0);
    CU_ASSERT(test_array.data != NULL);
    array_free(&test_array);
    CU_ASSERT(test_array.filled == 0);
}

void test_array_set(void) {
    array test_array;
    array_create(&test_array);
    array_set(&test_array, 0, test_data1);
    CU_ASSERT(test_array.filled == 1);
    CU_ASSERT(test_array.allocated_size >= 4);
    array_free(&test_array);
}

void test_array_get(void) {
    array test_array;
    array_create(&test_array);
    array_set(&test_array, 0, test_data1);

    CU_ASSERT_FATAL(array_get(&test_array, 0) == test_data1);
    CU_ASSERT_STRING_EQUAL(array_get(&test_array, 0), test_data1);
    CU_ASSERT(array_get(&test_array, 1) == NULL);

    array_free(&test_array);
}

void test_array_iter_next(void) {
    array test_array;
    array_create(&test_array);
    array_set(&test_array, 0, test_data1);
    array_set(&test_array, 1, test_data2);

    iterator it;
    array_iter_begin(&test_array, &it);
    CU_ASSERT_STRING_EQUAL(iter_next(&it), test_data1);
    CU_ASSERT_STRING_EQUAL(iter_next(&it), test_data2);
    CU_ASSERT(iter_next(&it) == NULL);

    array_free(&test_array);
}

void test_array_iter_prev(void) {
    array test_array;
    array_create(&test_array);
    array_set(&test_array, 0, test_data1);
    array_set(&test_array, 1, test_data2);

    iterator it;
    array_iter_end(&test_array, &it);
    CU_ASSERT_STRING_EQUAL(iter_prev(&it), test_data2);
    CU_ASSERT_STRING_EQUAL(iter_prev(&it), test_data1);
    CU_ASSERT(iter_prev(&it) == NULL);

    array_free(&test_array);
}

void array_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "Test for array create", test_array_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for array set", test_array_set) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for array get", test_array_get) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for array iter_next", test_array_iter_next) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for array iter_prev", test_array_iter_prev) == NULL) {
        return;
    }
}
