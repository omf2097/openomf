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
}
