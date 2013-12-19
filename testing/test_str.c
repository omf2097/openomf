#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <utils/str.h>

str m;

void test_str_cstr(void) {
    str d;
    str_create_from_cstr(&d, "testdata");
    CU_ASSERT(str_size(&d) == 8);
    CU_ASSERT_PTR_NOT_NULL(d.data);
    str_free(&d);
}

void test_str_mem(void) {
    str d;
    str_create_from_data(&d, "testdata", 8);
    CU_ASSERT(str_size(&d) == 8);
    CU_ASSERT_PTR_NOT_NULL(d.data);
    str_free(&d);
}

void test_str_create(void) {
    str_create(&m);
    CU_ASSERT(m.len == 0)
    CU_ASSERT_PTR_NULL(m.data);
}

void test_str_copy(void) {
    str src;
    str_create_from_cstr(&src, "testdata");
    str_copy(&m, &src);
    CU_ASSERT(str_size(&m) == 8);
    CU_ASSERT_PTR_NOT_NULL(m.data);
    str_free(&src);
}

void test_str_append(void) {
    str src;
    str_create_from_cstr(&src, "appended_X");
    str_append(&m, &src);
    CU_ASSERT(str_size(&m) == 18);
    CU_ASSERT_PTR_NOT_NULL(m.data);
    CU_ASSERT_NSTRING_EQUAL(m.data+8, "appended_X", 8);
    str_free(&src);
}

void test_str_prepend(void) {
    str src;
    str_create_from_cstr(&src, "prepended_");
    str_prepend(&m, &src);
    CU_ASSERT(str_size(&m) == 28);
    CU_ASSERT_PTR_NOT_NULL(m.data);
    CU_ASSERT_NSTRING_EQUAL(m.data, "prepended_", 10);
    str_free(&src);
}

void test_first_of(void) {
    size_t pos;
    CU_ASSERT(str_first_of(&m, '_', &pos));
    CU_ASSERT(pos == 9);
}

void test_last_of(void) {
    size_t pos;
    CU_ASSERT(str_last_of(&m, 'X', &pos));
    CU_ASSERT(pos == 27);
}

void test_equal(void) {
    str test;

    str_create_from_cstr(&test, "prepended_testdataappended_X");
    CU_ASSERT(str_equal(&m, &test));
    str_free(&test);

    str_create_from_cstr(&test, "awesome!");
    CU_ASSERT_FALSE(str_equal(&m, &test));
    str_free(&test);
}

void test_str_free(void) {
    str_free(&m);
    CU_ASSERT(m.len == 0)
    CU_ASSERT_PTR_NULL(m.data);
}

void str_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for string create from C string", test_str_cstr) == NULL) { return; }
    if(CU_add_test(suite, "Test for string create from buffer", test_str_mem) == NULL) { return; }

    if(CU_add_test(suite, "Test for string create", test_str_create) == NULL) { return; }
    if(CU_add_test(suite, "Test for string copy", test_str_copy) == NULL) { return; }
    if(CU_add_test(suite, "Test for string append", test_str_append) == NULL) { return; }
    if(CU_add_test(suite, "Test for string prepend", test_str_prepend) == NULL) { return; }

    if(CU_add_test(suite, "Test for string first of", test_first_of) == NULL) { return; }
    if(CU_add_test(suite, "Test for string last of", test_last_of) == NULL) { return; }
    if(CU_add_test(suite, "Test for string equals", test_last_of) == NULL) { return; }

    if(CU_add_test(suite, "Test for string free operation", test_str_free) == NULL) { return; }
}
