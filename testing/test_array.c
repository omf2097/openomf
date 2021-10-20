#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <utils/array.h>
#include <utils/iterator.h>
#include <stdlib.h>
#include "utils/allocator.h"

array test_array;
void *test_ptr;

#define TEST_STR_SIZE 8

void test_array_create(void) {
    array_create(&test_array);
    CU_ASSERT(test_array.filled == 0);
    CU_ASSERT(test_array.allocated_size != 0);
    CU_ASSERT(test_array.data != NULL);
}

void test_array_free(void) {
    array_free(&test_array);
    CU_ASSERT(test_array.filled == 0);
}

void test_array_set(void) {
    void *test_data = omf_calloc(TEST_STR_SIZE, 1);
    memset(test_data, 1, TEST_STR_SIZE);
    test_ptr = test_data;
    array_set(&test_array, 0, test_data);
    CU_ASSERT(test_array.filled == 1);
}

void test_array_get(void) {
    CU_ASSERT_FATAL(array_get(&test_array, 0) != NULL)
    CU_ASSERT(array_get(&test_array, 1) == NULL);
    CU_ASSERT_FATAL(array_get(&test_array, 0) == test_ptr);

    char *value = array_get(&test_array, 0);
    for(int i = 0; i < TEST_STR_SIZE; i++) {
        CU_ASSERT(value[i] == 1);
    }
}

void array_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for array create", test_array_create) == NULL) { return; }
    if(CU_add_test(suite, "Test for array set", test_array_set) == NULL) { return; }
    if(CU_add_test(suite, "Test for array get", test_array_get) == NULL) { return; }
    if(CU_add_test(suite, "Test for array free", test_array_free) == NULL) { return; }
}
