#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <utils/iterator.h>
#include <utils/vector.h>

#define TEST_VAL_COUNT 1000

vector test_vector;
static unsigned int test_values[TEST_VAL_COUNT];

void test_vector_create(void) {
    vector_create(&test_vector, sizeof(int));
    CU_ASSERT_PTR_NOT_NULL(test_vector.data);
    CU_ASSERT(vector_size(&test_vector) == 0);
    return;
}

void test_vector_free(void) {
    vector_free(&test_vector);
    CU_ASSERT_PTR_NULL(test_vector.data);
    return;
}

void test_vector_append(void) {
    for(int i = 0; i < TEST_VAL_COUNT / 2; i++) {
        CU_ASSERT(vector_append(&test_vector, (void *)&i) == 0);
        test_values[i] = TEST_VAL_COUNT - i;
        CU_ASSERT(vector_size(&test_vector) == i + 1);
    }
}

void test_vector_prepend(void) {
    for(int i = TEST_VAL_COUNT / 2; i < TEST_VAL_COUNT; i++) {
        CU_ASSERT(vector_prepend(&test_vector, (void *)&i) == 0);
        test_values[i] = TEST_VAL_COUNT - i;
        CU_ASSERT(vector_size(&test_vector) == i + 1);
    }
}

void test_vector_get(void) {
    for(int i = 0; i < TEST_VAL_COUNT; i++) {
        CU_ASSERT_PTR_NOT_NULL(vector_get(&test_vector, i));
    }

    // We try to fetch a too high an index; this should return NULL
    CU_ASSERT_PTR_NULL(vector_get(&test_vector, TEST_VAL_COUNT + 1));
}

void test_vector_iterator(void) {
    iterator it;
    vector_iter_begin(&test_vector, &it);
    int *val;

    // Read values from the vector
    while((val = iter_next(&it)) != NULL) {
        CU_ASSERT(test_values[*val] == TEST_VAL_COUNT - *val);
        test_values[*val] = 0;
    }

    // Make sure we got all values from the iterator
    for(int i = 0; i < TEST_VAL_COUNT; i++) {
        CU_ASSERT(test_values[i] == 0);
    }
}

void test_vector_delete(void) {
    iterator it;
    vector_iter_begin(&test_vector, &it);
    int *val;
    while((val = iter_next(&it)) != NULL) {
        CU_ASSERT(vector_delete(&test_vector, &it) == 0);
    }

    // Make sure the size is correct
    CU_ASSERT(vector_size(&test_vector) == 0);

    // Make sure the iterator returns nothing
    vector_iter_begin(&test_vector, &it);
    CU_ASSERT_PTR_NULL(iter_next(&it));
}

void test_vector_zero_size(void) {
    iterator it;
    vector_create_with_size(&test_vector, sizeof(int), 0);
    vector_iter_begin(&test_vector, &it);
    for(int i = 0; i < TEST_VAL_COUNT / 2; i++) {
        CU_ASSERT(vector_append(&test_vector, (void *)&i) == 0);
        test_values[i] = TEST_VAL_COUNT - i;
        CU_ASSERT(vector_size(&test_vector) == i + 1);
    }

    for(int i = 0; i < TEST_VAL_COUNT; i++) {
        CU_ASSERT_PTR_NOT_NULL(vector_get(&test_vector, i));
    }

    // We try to fetch a too high an index; this should return NULL
    CU_ASSERT_PTR_NULL(vector_get(&test_vector, TEST_VAL_COUNT + 1));
    vector_free(&test_vector);
}


void vector_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for vector create", test_vector_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector append", test_vector_append) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector prepend", test_vector_prepend) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector get", test_vector_get) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector iterator", test_vector_iterator) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector delete", test_vector_delete) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector free operation", test_vector_free) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector free operation", test_vector_free) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for zero size vector operation", test_vector_zero_size) == NULL) {
        return;
    }

}
