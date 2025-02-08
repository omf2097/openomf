#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
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
}
