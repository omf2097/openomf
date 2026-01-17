#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <utils/allocator.h>
#include <utils/iterator.h>
#include <utils/list.h>

const char *test_str = "abcdefgh";
const char *test_str_b = "aaaaaaaa";

void test_list_create(void) {
    list test_list;
    list_create(&test_list);
    CU_ASSERT(test_list.size == 0);
    CU_ASSERT(test_list.first == NULL);
    CU_ASSERT(test_list.last == NULL);
    list_free(&test_list);
    CU_ASSERT(test_list.size == 0);
}

void test_list_append(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, test_str, strlen(test_str) + 1);
    CU_ASSERT(test_list.size == 1);
    CU_ASSERT(test_list.first != NULL);
    CU_ASSERT(test_list.last != NULL);
    CU_ASSERT(test_list.last == test_list.first);
    CU_ASSERT_STRING_EQUAL(list_first(&test_list), test_str); // verify content

    // append second element
    list_append(&test_list, test_str_b, strlen(test_str_b) + 1);
    CU_ASSERT(test_list.size == 2);
    CU_ASSERT_STRING_EQUAL(list_first(&test_list), test_str);
    CU_ASSERT_STRING_EQUAL(list_last(&test_list), test_str_b);

    list_free(&test_list);
}

void test_list_prepend(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, test_str, strlen(test_str) + 1);
    list_prepend(&test_list, test_str_b, strlen(test_str_b) + 1);
    CU_ASSERT(test_list.size == 2);
    CU_ASSERT(test_list.first != NULL);
    CU_ASSERT(test_list.last != NULL);
    CU_ASSERT(test_list.last != test_list.first);

    // verify order: test_str_b should be first, test_str should be last
    CU_ASSERT_STRING_EQUAL(list_first(&test_list), test_str_b);
    CU_ASSERT_STRING_EQUAL(list_last(&test_list), test_str);

    list_free(&test_list);
}

void test_list_iter_next(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, test_str, strlen(test_str) + 1);
    list_append(&test_list, test_str_b, strlen(test_str_b) + 1);

    iterator it;
    list_iter_begin(&test_list, &it);
    CU_ASSERT_STRING_EQUAL(iter_next(&it), test_str);
    CU_ASSERT_STRING_EQUAL(iter_next(&it), test_str_b);
    CU_ASSERT(iter_next(&it) == NULL);

    list_free(&test_list);
}

void test_list_iter_peek(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, test_str, strlen(test_str) + 1);
    list_append(&test_list, test_str_b, strlen(test_str_b) + 1);

    iterator it;
    list_iter_begin(&test_list, &it);
    CU_ASSERT_STRING_EQUAL(iter_peek(&it), test_str);
    CU_ASSERT_STRING_EQUAL(iter_peek(&it), test_str); // Ensure iterator does not move

    list_free(&test_list);
}

void test_list_iter_prev(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, test_str, strlen(test_str) + 1);
    list_append(&test_list, test_str_b, strlen(test_str_b) + 1);

    iterator it;
    list_iter_end(&test_list, &it);
    CU_ASSERT_STRING_EQUAL(iter_prev(&it), test_str_b);
    CU_ASSERT_STRING_EQUAL(iter_prev(&it), test_str);
    CU_ASSERT(iter_prev(&it) == NULL);

    list_free(&test_list);
}

void test_list_delete(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, test_str, strlen(test_str) + 1);
    list_append(&test_list, test_str_b, strlen(test_str_b) + 1);
    list_append(&test_list, test_str, strlen(test_str) + 1);

    // Delete middle entry
    iterator it;
    list_iter_begin(&test_list, &it);
    iter_next(&it);
    iter_next(&it);
    list_delete(&test_list, &it);

    // Reset iterator and assert
    list_iter_begin(&test_list, &it);
    CU_ASSERT_STRING_EQUAL(iter_next(&it), test_str);
    CU_ASSERT_STRING_EQUAL(iter_next(&it), test_str);
    CU_ASSERT(iter_next(&it) == NULL);

    list_free(&test_list);
}

void test_list_iter_append(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, test_str, strlen(test_str) + 1);

    // Add to beginning
    iterator it;
    list_iter_begin(&test_list, &it);
    list_iter_append(&it, test_str_b, strlen(test_str_b) + 1);

    // Reset iterator and assert
    list_iter_begin(&test_list, &it);
    CU_ASSERT_STRING_EQUAL(iter_next(&it), test_str_b);
    CU_ASSERT_STRING_EQUAL(iter_next(&it), test_str);
    CU_ASSERT(iter_next(&it) == NULL);

    list_free(&test_list);
}

void test_list_get(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, test_str, strlen(test_str) + 1);
    CU_ASSERT(list_get(&test_list, 1) == NULL);
    CU_ASSERT(list_get(&test_list, 0) != NULL);
    CU_ASSERT(list_get(&test_list, 8192) == NULL);
    CU_ASSERT_STRING_EQUAL(list_get(&test_list, 0), test_str);
    list_free(&test_list);
}

void test_list_first_last(void) {
    list test_list;
    list_create(&test_list);

    // empty list
    CU_ASSERT(list_first(&test_list) == NULL);
    CU_ASSERT(list_last(&test_list) == NULL);

    // one element
    list_append(&test_list, test_str, strlen(test_str) + 1);
    CU_ASSERT_STRING_EQUAL(list_first(&test_list), test_str);
    CU_ASSERT_STRING_EQUAL(list_last(&test_list), test_str);
    CU_ASSERT(list_first(&test_list) == list_last(&test_list)); // same memory ptr

    // many elements
    list_append(&test_list, test_str_b, strlen(test_str_b) + 1);
    CU_ASSERT_STRING_EQUAL(list_first(&test_list), test_str);
    CU_ASSERT_STRING_EQUAL(list_last(&test_list), test_str_b);
    CU_ASSERT(list_first(&test_list) != list_last(&test_list)); // different memory ptr

    list_free(&test_list);
}

void test_list_pop_front(void) {
    list test_list;
    list_create(&test_list);

    // pop from empty list
    CU_ASSERT(list_pop_front(&test_list) == NULL);

    // pop one element
    list_append(&test_list, test_str, strlen(test_str) + 1);
    char *data = list_pop_front(&test_list);
    CU_ASSERT(data != NULL);
    CU_ASSERT_STRING_EQUAL(data, test_str);
    CU_ASSERT(list_size(&test_list) == 0);
    omf_free(data);

    // pop with multiple elements
    list_append(&test_list, test_str, strlen(test_str) + 1);
    list_append(&test_list, test_str_b, strlen(test_str_b) + 1);
    data = list_pop_front(&test_list);
    CU_ASSERT_STRING_EQUAL(data, test_str);
    CU_ASSERT(list_size(&test_list) == 1);
    omf_free(data);

    list_free(&test_list);
}

void test_list_pop_back(void) {
    list test_list;
    list_create(&test_list);

    // pop from empty list
    CU_ASSERT(list_pop_back(&test_list) == NULL);

    // pop single element
    list_append(&test_list, test_str, strlen(test_str) + 1);
    char *data = list_pop_back(&test_list);
    CU_ASSERT(data != NULL);
    CU_ASSERT_STRING_EQUAL(data, test_str);
    CU_ASSERT(list_size(&test_list) == 0);
    omf_free(data);

    // pop with multiple elements
    list_append(&test_list, test_str, strlen(test_str) + 1);
    list_append(&test_list, test_str_b, strlen(test_str_b) + 1);
    data = list_pop_back(&test_list);
    CU_ASSERT_STRING_EQUAL(data, test_str_b);
    CU_ASSERT(list_size(&test_list) == 1);
    omf_free(data);

    list_free(&test_list);
}

static int free_cb_call_count = 0;

static void test_free_cb(void *data) {
    (void)data;
    free_cb_call_count++;
}

void test_list_free_cb(void) {
    list test_list;
    free_cb_call_count = 0;
    list_create_cb(&test_list, test_free_cb);
    CU_ASSERT(test_list.free_cb == test_free_cb);

    // add elements
    list_append(&test_list, test_str, strlen(test_str) + 1);
    list_append(&test_list, test_str_b, strlen(test_str_b) + 1);
    list_append(&test_list, test_str, strlen(test_str) + 1);
    CU_ASSERT(free_cb_call_count == 0);

    // delete one element
    iterator it;
    list_iter_begin(&test_list, &it);
    iter_next(&it);
    list_delete(&test_list, &it);
    CU_ASSERT(free_cb_call_count == 1);

    // free remaining
    list_free(&test_list);
    CU_ASSERT(free_cb_call_count == 3);
}

void test_list_empty_operations(void) {
    list test_list;
    list_create(&test_list);

    // get from empty list
    CU_ASSERT(list_get(&test_list, 0) == NULL);
    CU_ASSERT(list_get(&test_list, 100) == NULL);

    // iterate empty list
    iterator it;
    list_iter_begin(&test_list, &it);
    CU_ASSERT(iter_next(&it) == NULL);
    CU_ASSERT(it.ended == 1);

    list_iter_end(&test_list, &it);
    CU_ASSERT(iter_prev(&it) == NULL);
    CU_ASSERT(it.ended == 1);

    // peek empty list
    list_iter_begin(&test_list, &it);
    CU_ASSERT(iter_peek(&it) == NULL);

    list_free(&test_list);
}

void test_list_delete_first(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, test_str, strlen(test_str) + 1);
    list_append(&test_list, test_str_b, strlen(test_str_b) + 1);

    // delete first element using forward iterator
    iterator it;
    list_iter_begin(&test_list, &it);
    iter_next(&it);
    list_delete(&test_list, &it);

    CU_ASSERT(list_size(&test_list) == 1);
    CU_ASSERT_STRING_EQUAL(list_first(&test_list), test_str_b);
    CU_ASSERT_STRING_EQUAL(list_last(&test_list), test_str_b);

    list_free(&test_list);
}

void test_list_delete_last(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, test_str, strlen(test_str) + 1);
    list_append(&test_list, test_str_b, strlen(test_str_b) + 1);

    // delete last element using reverse iterator
    iterator it;
    list_iter_end(&test_list, &it);
    iter_prev(&it);
    list_delete(&test_list, &it);

    CU_ASSERT(list_size(&test_list) == 1);
    CU_ASSERT_STRING_EQUAL(list_first(&test_list), test_str);
    CU_ASSERT_STRING_EQUAL(list_last(&test_list), test_str);

    list_free(&test_list);
}

void test_list_delete_single(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, test_str, strlen(test_str) + 1);

    // delete our only element
    iterator it;
    list_iter_begin(&test_list, &it);
    iter_next(&it);
    list_delete(&test_list, &it);

    CU_ASSERT(list_size(&test_list) == 0);
    CU_ASSERT(list_first(&test_list) == NULL);
    CU_ASSERT(list_last(&test_list) == NULL);

    list_free(&test_list);
}

void test_list_iter_append_middle(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, "first", 6);
    list_append(&test_list, "third", 6);

    // Insert middle
    iterator it;
    list_iter_begin(&test_list, &it);
    iter_next(&it); // now at "first"
    list_iter_append(&it, "second", 7);

    CU_ASSERT(list_size(&test_list) == 3);

    // verify order
    list_iter_begin(&test_list, &it);
    CU_ASSERT_STRING_EQUAL(iter_next(&it), "first");
    CU_ASSERT_STRING_EQUAL(iter_next(&it), "second");
    CU_ASSERT_STRING_EQUAL(iter_next(&it), "third");

    list_free(&test_list);
}

void test_list_iter_append_end(void) {
    list test_list;
    list_create(&test_list);
    list_append(&test_list, "first", 6);

    // insert at end (after last element)
    iterator it;
    list_iter_begin(&test_list, &it);
    iter_next(&it); // now at "first" (which is also last)
    list_iter_append(&it, "second", 7);

    CU_ASSERT(list_size(&test_list) == 2);
    CU_ASSERT_STRING_EQUAL(list_first(&test_list), "first");
    CU_ASSERT_STRING_EQUAL(list_last(&test_list), "second");

    list_free(&test_list);
}

void test_list_get_backward(void) {
    list test_list;
    list_create(&test_list);
    for(int i = 0; i < 10; i++) {
        list_append(&test_list, &i, sizeof(int));
    }

    // test get with index in first half (forward iteration)
    const int *val = list_get(&test_list, 2);
    CU_ASSERT(val != NULL);
    CU_ASSERT(*val == 2);

    // test get with index in second half (backward iteration, i >= size/2)
    val = list_get(&test_list, 7);
    CU_ASSERT(val != NULL);
    CU_ASSERT(*val == 7);

    list_free(&test_list);
}

void list_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for list create", test_list_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list append", test_list_append) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list prepend", test_list_prepend) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list iter_next", test_list_iter_next) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list iter_peek", test_list_iter_peek) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list iter_prev", test_list_iter_prev) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list get", test_list_get) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list delete", test_list_delete) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list iter_append", test_list_iter_append) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list first and last", test_list_first_last) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list pop_front", test_list_pop_front) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list pop_back", test_list_pop_back) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list free_cb", test_list_free_cb) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list empty operations", test_list_empty_operations) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list delete first", test_list_delete_first) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list delete last", test_list_delete_last) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list delete single", test_list_delete_single) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list iter_append middle", test_list_iter_append_middle) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list iter_append end", test_list_iter_append_end) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for list get backward", test_list_get_backward) == NULL) {
        return;
    }
}
