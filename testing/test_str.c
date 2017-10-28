#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <utils/str.h>

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
    str m;
    str_create(&m);
    CU_ASSERT(m.len == 0);
    CU_ASSERT_PTR_NOT_NULL(m.data);
    str_free(&m);
}

void test_str_free(void) {
    str m;
    str_create(&m);
    str_free(&m);
    CU_ASSERT(m.len == 0)
    CU_ASSERT_PTR_NULL(m.data);
}

void test_str_copy(void) {
    str src;
    str dst;
    str_create_from_cstr(&src, "testdata");
    str_create_from_cstr(&dst, "base_string");
    str_copy(&dst, &src);
    CU_ASSERT(str_size(&dst) == 8);
    CU_ASSERT_PTR_NOT_NULL(dst.data);
    str_free(&src);
    str_free(&dst);
}

void test_str_slice(void) {
    str src;
    str dst;

    str_create_from_cstr(&src, "test-data-a");
    str_create(&dst);

    str_slice(&dst, &src, 0, 9);

    CU_ASSERT(dst.len == 9);
    CU_ASSERT_NSTRING_EQUAL(dst.data, "test-data", 9);

    str_free(&src);
    str_free(&dst);
}

void test_str_append(void) {
    str dst;
    str src;
    str_create_from_cstr(&src, "appended_string");
    str_create_from_cstr(&dst, "base_string");
    str_append(&dst, &src);
    CU_ASSERT(str_size(&dst) == 26);
    CU_ASSERT_PTR_NOT_NULL(dst.data);
    CU_ASSERT_NSTRING_EQUAL(dst.data, "base_string", 11);
    CU_ASSERT_NSTRING_EQUAL(dst.data+11, "appended_string", 15);
    CU_ASSERT(dst.data[dst.len] == 0);
    str_free(&src);
    str_free(&dst);
}

void test_str_append_c(void) {
    str dst;
    str_create_from_cstr(&dst, "base_string");
    str_append_c(&dst, "appended_string");
    CU_ASSERT(str_size(&dst) == 26);
    CU_ASSERT_PTR_NOT_NULL(dst.data);
    CU_ASSERT_NSTRING_EQUAL(dst.data, "base_string", 11);
    CU_ASSERT_NSTRING_EQUAL(dst.data+11, "appended_string", 15);
    CU_ASSERT(dst.data[dst.len] == 0);
    str_free(&dst);
}

void test_str_prepend(void) {
    str dst;
    str src;
    str_create_from_cstr(&src, "prepended_string");
    str_create_from_cstr(&dst, "base_string");
    str_prepend(&dst, &src);
    CU_ASSERT(str_size(&dst) == 27);
    CU_ASSERT_PTR_NOT_NULL(dst.data);
    CU_ASSERT_NSTRING_EQUAL(dst.data, "prepended_string", 16);
    CU_ASSERT_NSTRING_EQUAL(dst.data+16, "base_string", 11);
    CU_ASSERT(dst.data[dst.len] == 0);
    str_free(&src);
    str_free(&dst);
}

void test_str_prepend_c(void) {
    str dst;
    str_create_from_cstr(&dst, "base_string");
    str_prepend_c(&dst, "prepended_string");
    CU_ASSERT(str_size(&dst) == 27);
    CU_ASSERT_PTR_NOT_NULL(dst.data);
    CU_ASSERT_NSTRING_EQUAL(dst.data, "prepended_string", 16);
    CU_ASSERT_NSTRING_EQUAL(dst.data+16, "base_string", 11);
    CU_ASSERT(dst.data[dst.len] == 0);
    str_free(&dst);
}

void test_first_of(void) {
    size_t pos;
    str dst;
    str_create_from_cstr(&dst, "base_string");
    CU_ASSERT(str_first_of(&dst, '_', &pos));
    CU_ASSERT(pos == 4);
    str_free(&dst);
}

void test_last_of(void) {
    size_t pos;
    str dst;
    str_create_from_cstr(&dst, "base_string_X");
    CU_ASSERT(str_last_of(&dst, 'X', &pos));
    CU_ASSERT(pos == 12);
    str_free(&dst);
}

void test_equal(void) {
    str test_a;
    str test_b;

    str_create_from_cstr(&test_a, "prepended_testdataappended_X");

    str_create_from_cstr(&test_b, "prepended_testdataappended_X");
    CU_ASSERT(str_equal(&test_a, &test_b) == 1);
    str_free(&test_b);

    str_create_from_cstr(&test_b, "awesome!");
    CU_ASSERT(str_equal(&test_a, &test_b) == 0);
    str_free(&test_b);

    str_free(&test_a);
}

void test_printf(void) {
    str test;
    str_create(&test);
    str_printf(&test, "%s, %d\n", "this is a test", 100);

    CU_ASSERT(test.len == 20);
    CU_ASSERT_PTR_NOT_NULL(test.data);
    CU_ASSERT(test.data[20] == 0);

    str_printf(&test, "over %d thousaaand!\n", 9);

    CU_ASSERT(test.len == 39);
    CU_ASSERT_PTR_NOT_NULL(test.data);
    CU_ASSERT(test.data[39] == 0);
}

void str_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for string create from C string", test_str_cstr) == NULL) { return; }
    if(CU_add_test(suite, "Test for string create from buffer", test_str_mem) == NULL) { return; }
    if(CU_add_test(suite, "Test for string create", test_str_create) == NULL) { return; }
    if(CU_add_test(suite, "Test for string free operation", test_str_free) == NULL) { return; }

    if(CU_add_test(suite, "Test for string copy", test_str_copy) == NULL) { return; }
    if(CU_add_test(suite, "Test for string slice", test_str_slice) == NULL) { return; }
    if(CU_add_test(suite, "Test for string append", test_str_append) == NULL) { return; }
    if(CU_add_test(suite, "Test for string append cstr", test_str_append_c) == NULL) { return; }
    if(CU_add_test(suite, "Test for string prepend", test_str_prepend) == NULL) { return; }
    if(CU_add_test(suite, "Test for string prepend cstr", test_str_prepend_c) == NULL) { return; }

    if(CU_add_test(suite, "Test for string first of", test_first_of) == NULL) { return; }
    if(CU_add_test(suite, "Test for string last of", test_last_of) == NULL) { return; }
    if(CU_add_test(suite, "Test for string equals", test_last_of) == NULL) { return; }

    if(CU_add_test(suite, "Test for string printf", test_printf) == NULL) { return; }
}
