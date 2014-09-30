#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <utils/list.h>
#include <utils/iterator.h>

list test_list;
const char *test_str = "abcdefgh";
const char *test_str_b = "aaaaaaaa";

void test_list_create(void) {
    list_create(&test_list);
    CU_ASSERT(test_list.size == 0);
    CU_ASSERT(test_list.first == NULL);
    CU_ASSERT(test_list.last == NULL);
}

void test_list_free(void) {
    list_free(&test_list);
    CU_ASSERT(test_list.size == 0);
}

void test_list_append(void) {
    list_append(&test_list, test_str, strlen(test_str)+1);
    CU_ASSERT(test_list.size == 1);
    CU_ASSERT(test_list.first != NULL);
    CU_ASSERT(test_list.last != NULL);
    CU_ASSERT(test_list.last == test_list.first);
}

void test_list_prepend(void) {
    list_prepend(&test_list, test_str_b, strlen(test_str_b)+1);
    CU_ASSERT(test_list.size == 2);
    CU_ASSERT(test_list.first != NULL);
    CU_ASSERT(test_list.last != NULL);
    CU_ASSERT(test_list.last != test_list.first);
}

void test_list_get(void) {
    CU_ASSERT_FATAL(list_get(&test_list, 1) != NULL);
    CU_ASSERT_FATAL(list_get(&test_list, 0) != NULL);
    CU_ASSERT(list_get(&test_list, 2) == NULL);
    CU_ASSERT(list_get(&test_list, 8192) == NULL);

    CU_ASSERT_STRING_EQUAL(list_get(&test_list, 0), test_str_b);
    CU_ASSERT_STRING_EQUAL(list_get(&test_list, 1), test_str);
}

void list_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for list create", test_list_create) == NULL) { return; }
    if(CU_add_test(suite, "Test for list append", test_list_append) == NULL) { return; }
    if(CU_add_test(suite, "Test for list prepend", test_list_prepend) == NULL) { return; }
    if(CU_add_test(suite, "Test for list get", test_list_get) == NULL) { return; }
    if(CU_add_test(suite, "Test for list free", test_list_free) == NULL) { return; }
}
