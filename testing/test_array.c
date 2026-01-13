#include "utils/allocator.h"
#include <CUnit/CUnit.h>
#include <utils/array.h>
#include <utils/iterator.h>

static const char *test_data1 = "TEST1";
static const char *test_data2 = "TEST2";
static const char *test_data3 = "TEST3";
static const char *test_data4 = "TEST4";
static const char *test_data5 = "TEST5";

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

void test_array_sparse(void) {
    array test_array;
    array_create(&test_array);

    // Set sparse indices
    array_set(&test_array, 0, test_data1);
    array_set(&test_array, 2, test_data2);
    array_set(&test_array, 4, test_data3);

    CU_ASSERT_EQUAL(test_array.filled, 3);
    CU_ASSERT_PTR_EQUAL(array_get(&test_array, 0), test_data1);
    CU_ASSERT_PTR_NULL(array_get(&test_array, 1));
    CU_ASSERT_PTR_EQUAL(array_get(&test_array, 2), test_data2);
    CU_ASSERT_PTR_NULL(array_get(&test_array, 3));
    CU_ASSERT_PTR_EQUAL(array_get(&test_array, 4), test_data3);

    array_free(&test_array);
}

void test_array_growth(void) {
    array test_array;
    array_create(&test_array);

    const unsigned int initial_size = test_array.allocated_size;
    array_set(&test_array, 100, test_data1); // Make it grow

    CU_ASSERT(test_array.allocated_size > initial_size);
    CU_ASSERT(test_array.allocated_size >= 101);
    CU_ASSERT_PTR_EQUAL(array_get(&test_array, 100), test_data1);

    array_free(&test_array);
}

void test_array_get_out_of_bounds(void) {
    array test_array;
    array_create(&test_array);

    array_set(&test_array, 0, test_data1);
    CU_ASSERT_PTR_NULL(array_get(&test_array, 10000)); // Nonexistent

    array_free(&test_array);
}

void test_array_iter_empty(void) {
    array test_array;
    array_create(&test_array);

    iterator it;
    array_iter_begin(&test_array, &it);
    CU_ASSERT_PTR_NULL(iter_next(&it)); // Empty, returns NULL

    array_free(&test_array);
}

void test_array_iter_skip_nulls(void) {
    array test_array;
    array_create(&test_array);
    array_set(&test_array, 0, test_data1);
    array_set(&test_array, 5, test_data2);
    array_set(&test_array, 10, test_data3);

    iterator it;
    array_iter_begin(&test_array, &it);

    // Iterator should skip NULLs
    CU_ASSERT_PTR_EQUAL(iter_next(&it), test_data1);
    CU_ASSERT_PTR_EQUAL(iter_next(&it), test_data2);
    CU_ASSERT_PTR_EQUAL(iter_next(&it), test_data3);
    CU_ASSERT_PTR_NULL(iter_next(&it));

    array_free(&test_array);
}

void test_array_iter_reverse_skip_nulls(void) {
    array test_array;
    array_create(&test_array);

    // Set sparse elements
    array_set(&test_array, 0, test_data1);
    array_set(&test_array, 5, test_data2);
    array_set(&test_array, 10, test_data3);

    iterator it;
    array_iter_end(&test_array, &it);

    // Iterator should skip NULLs
    CU_ASSERT_PTR_EQUAL(iter_prev(&it), test_data3);
    CU_ASSERT_PTR_EQUAL(iter_prev(&it), test_data2);
    CU_ASSERT_PTR_EQUAL(iter_prev(&it), test_data1);
    CU_ASSERT_PTR_NULL(iter_prev(&it));

    array_free(&test_array);
}

void test_array_overwrite(void) {
    array test_array;
    array_create(&test_array);

    array_set(&test_array, 0, test_data1);
    CU_ASSERT_PTR_EQUAL(array_get(&test_array, 0), test_data1);

    array_set(&test_array, 0, test_data2);
    CU_ASSERT_PTR_EQUAL(array_get(&test_array, 0), test_data2);

    array_free(&test_array);
}

void test_array_dense(void) {
    array test_array;
    array_create(&test_array);

    array_set(&test_array, 0, test_data1);
    array_set(&test_array, 1, test_data2);
    array_set(&test_array, 2, test_data3);
    array_set(&test_array, 3, test_data4);
    array_set(&test_array, 4, test_data5);

    CU_ASSERT_EQUAL(test_array.filled, 5);

    for(int i = 0; i < 5; i++) {
        CU_ASSERT_PTR_NOT_NULL(array_get(&test_array, i));
    }

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
    if(CU_add_test(suite, "Test for array sparse", test_array_sparse) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for array growth", test_array_growth) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for array get out of bounds", test_array_get_out_of_bounds) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for array iter empty", test_array_iter_empty) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for array iter skip nulls", test_array_iter_skip_nulls) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for array iter reverse skip nulls", test_array_iter_reverse_skip_nulls) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for array overwrite", test_array_overwrite) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for array dense", test_array_dense) == NULL) {
        return;
    }
}
