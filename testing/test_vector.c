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

void test_vector_back_empty(void) {
    vector test_vector;
    vector_create(&test_vector, sizeof(int));

    CU_ASSERT_PTR_NULL(vector_back(&test_vector));

    vector_free(&test_vector);
}

void test_vector_clone(void) {
    int values[3] = {1, 2, 3};
    vector src;
    vector_create(&src, sizeof(int));
    vector_append(&src, &values[0]);
    vector_append(&src, &values[1]);
    vector_append(&src, &values[2]);

    vector dst;
    vector_clone(&dst, &src);

    CU_ASSERT(vector_size(&dst) == vector_size(&src));
    CU_ASSERT(*(int *)vector_get(&dst, 0) == values[0]);
    CU_ASSERT(*(int *)vector_get(&dst, 1) == values[1]);
    CU_ASSERT(*(int *)vector_get(&dst, 2) == values[2]);

    vector_free(&src);
    vector_free(&dst);
}

void test_vector_clone_empty(void) {
    vector src;
    vector_create(&src, sizeof(int));

    vector dst;
    vector_clone(&dst, &src);

    CU_ASSERT(vector_size(&dst) == 0);
    CU_ASSERT_PTR_NULL(dst.data);

    vector_free(&src);
    vector_free(&dst);
}

void test_vector_clear(void) {
    int values[3] = {1, 2, 3};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    vector_append(&test_vector, &values[1]);
    vector_append(&test_vector, &values[2]);

    CU_ASSERT(vector_size(&test_vector) == 3);
    vector_clear(&test_vector);

    // Size should be 0, but data pointer should still be valid (not freed)
    CU_ASSERT(vector_size(&test_vector) == 0);
    CU_ASSERT_PTR_NOT_NULL(test_vector.data);

    vector_free(&test_vector);
}

void test_vector_set(void) {
    int values[2] = {1, 2};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    vector_append(&test_vector, &values[1]);

    const int new_value = 99;
    CU_ASSERT(vector_set(&test_vector, 0, &new_value) == 0);
    CU_ASSERT(*(int *)vector_get(&test_vector, 0) == new_value);
    CU_ASSERT(*(int *)vector_get(&test_vector, 1) == values[1]);

    // Out of bounds returns 1
    CU_ASSERT(vector_set(&test_vector, 2, &new_value) == 1);
    CU_ASSERT(vector_set(&test_vector, 100, &new_value) == 1);

    vector_free(&test_vector);
}

void test_vector_delete_at(void) {
    int values[4] = {1, 2, 3, 4};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    vector_append(&test_vector, &values[1]);
    vector_append(&test_vector, &values[2]);
    vector_append(&test_vector, &values[3]);

    // Delete from middle - should memmove
    CU_ASSERT(vector_delete_at(&test_vector, 1) == 0);
    CU_ASSERT(vector_size(&test_vector) == 3);
    CU_ASSERT(*(int *)vector_get(&test_vector, 0) == values[0]);
    CU_ASSERT(*(int *)vector_get(&test_vector, 1) == values[2]);
    CU_ASSERT(*(int *)vector_get(&test_vector, 2) == values[3]);

    // Delete from end - no move needed
    CU_ASSERT(vector_delete_at(&test_vector, 2) == 0);
    CU_ASSERT(vector_size(&test_vector) == 2);

    // Delete from beginning
    CU_ASSERT(vector_delete_at(&test_vector, 0) == 0);
    CU_ASSERT(vector_size(&test_vector) == 1);
    CU_ASSERT(*(int *)vector_get(&test_vector, 0) == values[2]);

    // Out of bounds should return 1
    CU_ASSERT(vector_delete_at(&test_vector, 1) == 1);
    CU_ASSERT(vector_delete_at(&test_vector, 100) == 1);

    vector_free(&test_vector);
}

void test_vector_swapdelete_at_last(void) {
    int values[3] = {1, 2, 3};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    vector_append(&test_vector, &values[1]);
    vector_append(&test_vector, &values[2]);

    // Corner case: delete last element (no swap)
    CU_ASSERT(vector_swapdelete_at(&test_vector, 2) == 0);
    CU_ASSERT(vector_size(&test_vector) == 2);
    CU_ASSERT(*(int *)vector_get(&test_vector, 0) == values[0]);
    CU_ASSERT(*(int *)vector_get(&test_vector, 1) == values[1]);

    // Out of bounds should return 1
    CU_ASSERT(vector_swapdelete_at(&test_vector, 2) == 1);
    CU_ASSERT(vector_swapdelete_at(&test_vector, 100) == 1);

    vector_free(&test_vector);
}

static int compare_ints(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

void test_vector_sort(void) {
    int values[5] = {5, 2, 4, 1, 3};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    for(int i = 0; i < 5; i++) {
        vector_append(&test_vector, &values[i]);
    }

    vector_sort(&test_vector, compare_ints);

    CU_ASSERT(*(int *)vector_get(&test_vector, 0) == 1);
    CU_ASSERT(*(int *)vector_get(&test_vector, 1) == 2);
    CU_ASSERT(*(int *)vector_get(&test_vector, 2) == 3);
    CU_ASSERT(*(int *)vector_get(&test_vector, 3) == 4);
    CU_ASSERT(*(int *)vector_get(&test_vector, 4) == 5);

    vector_free(&test_vector);
}

void test_vector_append_ptr(void) {
    vector test_vector;
    vector_create(&test_vector, sizeof(int));

    int *ptr1 = vector_append_ptr(&test_vector);
    *ptr1 = 42;

    int *ptr2 = vector_append_ptr(&test_vector);
    *ptr2 = 99;

    CU_ASSERT(vector_size(&test_vector) == 2);
    CU_ASSERT(*(int *)vector_get(&test_vector, 0) == 42);
    CU_ASSERT(*(int *)vector_get(&test_vector, 1) == 99);

    vector_free(&test_vector);
}

static int free_cb_call_count = 0;

static void test_free_cb(void *data) {
    (void)data;
    free_cb_call_count++;
}

void test_vector_free_callback(void) {
    free_cb_call_count = 0;

    vector test_vector;
    vector_create_cb(&test_vector, sizeof(int), test_free_cb);

    int values[5] = {1, 2, 3, 4, 5};
    for(int i = 0; i < 5; i++) {
        vector_append(&test_vector, &values[i]);
    }

    // pop should call free_cb once
    vector_pop(&test_vector);
    CU_ASSERT(free_cb_call_count == 1);

    // delete_at should call free_cb
    vector_delete_at(&test_vector, 1);
    CU_ASSERT(free_cb_call_count == 2);

    // swapdelete_at should call free_cb
    vector_swapdelete_at(&test_vector, 0);
    CU_ASSERT(free_cb_call_count == 3);

    // clear should call free_cb for remaining 2 elements
    vector_clear(&test_vector);
    CU_ASSERT(free_cb_call_count == 5);

    // Append more to test free
    vector_append(&test_vector, &values[0]);
    vector_free(&test_vector);
    CU_ASSERT(free_cb_call_count == 6);
}

void test_vector_pop_empty(void) {
    vector test_vector;
    vector_create(&test_vector, sizeof(int));

    // Should be a no-op, not crash
    vector_pop(&test_vector);
    CU_ASSERT(vector_size(&test_vector) == 0);

    vector_free(&test_vector);
}

void test_vector_delete_empty(void) {
    vector test_vector;
    vector_create(&test_vector, sizeof(int));

    iterator it;
    vector_iter_begin(&test_vector, &it);

    CU_ASSERT(vector_delete(&test_vector, &it) == 1);

    vector_free(&test_vector);
}

void test_vector_delete_reverse(void) {
    int values[3] = {1, 2, 3};
    vector test_vector;
    vector_create(&test_vector, sizeof(int));
    vector_append(&test_vector, &values[0]);
    vector_append(&test_vector, &values[1]);
    vector_append(&test_vector, &values[2]);

    iterator it;
    vector_iter_end(&test_vector, &it);
    int count = 0;
    int *val;
    foreach_reverse(it, val) {
        CU_ASSERT(vector_delete(&test_vector, &it) == 0);
        count++;
    }

    CU_ASSERT(count == 3);
    CU_ASSERT(vector_size(&test_vector) == 0);

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
    if(CU_add_test(suite, "Test for vector back on empty", test_vector_back_empty) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector clone", test_vector_clone) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector clone empty", test_vector_clone_empty) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector clear", test_vector_clear) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector set", test_vector_set) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector delete_at", test_vector_delete_at) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector swapdelete_at last element", test_vector_swapdelete_at_last) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector sort", test_vector_sort) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector append_ptr", test_vector_append_ptr) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector free callback", test_vector_free_callback) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector pop on empty", test_vector_pop_empty) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector delete on empty", test_vector_delete_empty) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for vector delete reverse iteration", test_vector_delete_reverse) == NULL) {
        return;
    }
}
