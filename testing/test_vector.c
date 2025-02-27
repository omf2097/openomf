#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <utils/iterator.h>
#include <utils/vector.h>

void test_vector_create(void) {
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    CU_ASSERT_PTR_NOT_NULL(test_vector.data);
    CU_ASSERT(vector_size(&test_vector) == 0);
    vector_free(&test_vector);
    CU_ASSERT_PTR_NULL(test_vector.data);
}

void test_vector_append(void) {
    int values[2] = {1, 2};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    vector_append(&test_vector, &values[1]);

    CU_ASSERT(vector_size(&test_vector) == 2);
    CU_ASSERT(*(int *)vector_get(&test_vector, 0) == values[0]);
    CU_ASSERT(*(int *)vector_get(&test_vector, 1) == values[1]);
    CU_ASSERT(vector_get(&test_vector, 2) == NULL);

    vector_free(&test_vector);
}

void test_vector_get(void) {
    int values[1] = {1};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    CU_ASSERT(*(int *)vector_get(&test_vector, 0) == values[0]);
    CU_ASSERT(vector_get(&test_vector, 1) == NULL);

    vector_free(&test_vector);
}

void test_vector_iter_next(void) {
    int values[2] = {1, 2};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    vector_append(&test_vector, &values[1]);

    iterator it;
    vector_iter_begin(&test_vector, &it);
    CU_ASSERT(*(int *)iter_next(&it) == values[0]);
    CU_ASSERT(*(int *)iter_next(&it) == values[1]);
    CU_ASSERT(iter_next(&it) == NULL);

    vector_free(&test_vector);
}

void test_vector_iter_prev(void) {
    int values[2] = {1, 2};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    vector_append(&test_vector, &values[1]);

    iterator it;
    vector_iter_end(&test_vector, &it);
    CU_ASSERT(*(int *)iter_prev(&it) == values[1]);
    CU_ASSERT(*(int *)iter_prev(&it) == values[0]);
    CU_ASSERT(iter_prev(&it) == NULL);

    vector_free(&test_vector);
}

void test_vector_delete(void) {
    int values[2] = {1, 2};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    vector_append(&test_vector, &values[1]);

    iterator it;
    vector_iter_begin(&test_vector, &it);
    int *val;
    foreach(it, val) {
        CU_ASSERT(vector_delete(&test_vector, &it) == 0);
    }

    // Make sure the size is correct
    CU_ASSERT(vector_size(&test_vector) == 0);

    // Make sure the iterator returns nothing
    vector_iter_begin(&test_vector, &it);
    CU_ASSERT_PTR_NULL(iter_next(&it));

    vector_free(&test_vector);
}

void test_vector_swap_delete_at(void) {
    int values[2] = {1, 2};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    vector_append(&test_vector, &values[1]);

    vector_swapdelete_at(&test_vector, 0);

    CU_ASSERT(vector_size(&test_vector) == 1);
    iterator it;
    vector_iter_begin(&test_vector, &it);
    CU_ASSERT(*(int *)iter_next(&it) == values[1]);
    CU_ASSERT_PTR_NULL(iter_next(&it));

    vector_free(&test_vector);
}

void test_vector_pop(void) {
    int values[2] = {1, 2};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    vector_append(&test_vector, &values[1]);

    vector_pop(&test_vector);

    CU_ASSERT(vector_size(&test_vector) == 1);
    iterator it;
    vector_iter_begin(&test_vector, &it);
    CU_ASSERT(*(int *)iter_next(&it) == values[0]);
    CU_ASSERT_PTR_NULL(iter_next(&it));

    vector_free(&test_vector);
}

void test_vector_back(void) {
    int values[2] = {1, 2};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    vector_append(&test_vector, &values[1]);

    const void *ptr = vector_back(&test_vector);
    CU_ASSERT(*(int *)ptr == values[1]);

    vector_free(&test_vector);
}

void test_vector_zero_size(void) {
    iterator it;
    vector zero_vector;
    vector_create_with_size(&zero_vector, sizeof(int), 0);
    vector_iter_begin(&zero_vector, &it);

    for(unsigned i = 0; i < 100; i++) {
        vector_append(&zero_vector, (void *)&i);
        CU_ASSERT(vector_size(&zero_vector) == i + 1);
    }

    for(unsigned i = 0; i < 100; i++) {
        CU_ASSERT_PTR_NOT_NULL(vector_get(&zero_vector, i));
    }

    // We try to fetch a too high an index; this should return NULL
    CU_ASSERT_PTR_NULL(vector_get(&zero_vector, 101));
    vector_free(&zero_vector);
}

void vector_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for vector create", test_vector_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector append", test_vector_append) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector get", test_vector_get) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector iter_next", test_vector_iter_next) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector iter_prev", test_vector_iter_prev) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector delete", test_vector_delete) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector swap_delete_at", test_vector_swap_delete_at) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for zero size vector operation", test_vector_zero_size) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector pop", test_vector_pop) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector back", test_vector_back) == NULL) {
        return;
    }
}
