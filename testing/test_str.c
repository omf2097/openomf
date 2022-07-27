#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <utils/str.h>

void test_str_create(void) {
    str m;
    str_create(&m);
    CU_ASSERT(m.len == 0);
    CU_ASSERT_PTR_NOT_NULL(m.data);
    CU_ASSERT(m.data[0] == 0);
    str_free(&m);
}

void test_str_from(void) {
    str src;
    str_from_c(&src, "testdata");

    str d;
    str_from(&d, &src);
    CU_ASSERT(str_size(&d) == 8);
    CU_ASSERT_PTR_NOT_NULL(d.data);
    CU_ASSERT(d.data[8] == 0);

    str_free(&d);
    str_free(&src);
}

void test_str_from_c(void) {
    str d;
    str_from_c(&d, "testdata");
    CU_ASSERT(str_size(&d) == 8);
    CU_ASSERT_PTR_NOT_NULL(d.data);
    CU_ASSERT(d.data[8] == 0);
    str_free(&d);
}

void test_str_from_buf(void) {
    str d;
    str_from_buf(&d, "testdata", 8);
    CU_ASSERT(str_size(&d) == 8);
    CU_ASSERT_PTR_NOT_NULL(d.data);
    CU_ASSERT(d.data[8] == 0);
    str_free(&d);
}

void test_str_from_format(void) {
    str d;
    str_from_format(&d, "%s, %d\n", "this is a test", 100);

    CU_ASSERT_STRING_EQUAL(str_c(&d), "this is a test, 100\n")
    CU_ASSERT(d.len == 20);
    CU_ASSERT_PTR_NOT_NULL(d.data);
    CU_ASSERT(d.data[20] == 0);

    str_free(&d);
}

void test_str_from_slice(void) {
    str src;
    str dst;

    str_from_c(&src, "test-data-a");
    str_from_slice(&dst, &src, 0, 9);

    CU_ASSERT(dst.len == 9);
    CU_ASSERT_NSTRING_EQUAL(dst.data, "test-data", 9);

    str_free(&src);
    str_free(&dst);
}

void test_str_free(void) {
    str m;
    str_create(&m);
    str_free(&m);
    CU_ASSERT(m.len == 0)
    CU_ASSERT_PTR_NULL(m.data);
}

void test_str_toupper(void) {
    str d;
    str_from_c(&d, "test-string");
    str_toupper(&d);

    CU_ASSERT_NSTRING_EQUAL(d.data, "TEST-STRING", 11);
    CU_ASSERT(d.data[11] == 0);

    str_free(&d);
}

void test_str_tolower(void) {
    str d;
    str_from_c(&d, "TEST-STRING");
    str_tolower(&d);

    CU_ASSERT_NSTRING_EQUAL(d.data, "test-string", 11);
    CU_ASSERT(d.data[11] == 0);

    str_free(&d);
}

void test_str_rstrip(void) {
    str d;
    str_from_c(&d, "  test  ");
    str_rstrip(&d);

    CU_ASSERT_NSTRING_EQUAL(d.data, "  test", 6);
    CU_ASSERT(d.data[d.len] == 0);

    str_free(&d);
}

void test_str_lstrip(void) {
    str d;
    str_from_c(&d, "  test  ");
    str_lstrip(&d);

    CU_ASSERT_NSTRING_EQUAL(d.data, "test  ", 6);
    CU_ASSERT(d.data[d.len] == 0);

    str_free(&d);
}

void test_str_strip(void) {
    str d;
    str_from_c(&d, "  test  ");
    str_strip(&d);

    CU_ASSERT_NSTRING_EQUAL(d.data, "test", 4);
    CU_ASSERT(d.data[d.len] == 0);

    str_free(&d);
}

void test_str_append(void) {
    str dst;
    str src;
    str_from_c(&src, "appended_string");
    str_from_c(&dst, "base_string");
    str_append(&dst, &src);
    CU_ASSERT(str_size(&dst) == 26);
    CU_ASSERT_PTR_NOT_NULL(dst.data);
    CU_ASSERT_NSTRING_EQUAL(dst.data, "base_string", 11);
    CU_ASSERT_NSTRING_EQUAL(dst.data + 11, "appended_string", 15);
    CU_ASSERT(dst.data[dst.len] == 0);
    str_free(&src);
    str_free(&dst);
}

void test_str_append_c(void) {
    str dst;
    str_from_c(&dst, "base_string");
    str_append_c(&dst, "appended_string");
    CU_ASSERT(str_size(&dst) == 26);
    CU_ASSERT_PTR_NOT_NULL(dst.data);
    CU_ASSERT_NSTRING_EQUAL(dst.data, "base_string", 11);
    CU_ASSERT_NSTRING_EQUAL(dst.data + 11, "appended_string", 15);
    CU_ASSERT(dst.data[dst.len] == 0);
    str_free(&dst);
}

void test_str_append_buf(void) {
    str dst;
    str_from_c(&dst, "base_string");
    str_append_buf(&dst, "appended_string", 15);
    CU_ASSERT(str_size(&dst) == 26);
    CU_ASSERT_PTR_NOT_NULL(dst.data);
    CU_ASSERT_NSTRING_EQUAL(dst.data, "base_string", 11);
    CU_ASSERT_NSTRING_EQUAL(dst.data + 11, "appended_string", 15);
    CU_ASSERT(dst.data[dst.len] == 0);
    str_free(&dst);
}

void test_str_first_of(void) {
    size_t pos = 0;
    str dst;
    str_from_c(&dst, "base_string");
    CU_ASSERT(str_first_of(&dst, '_', &pos));
    CU_ASSERT(pos == 4);
    str_free(&dst);
}

void test_str_last_of_near(void) {
    size_t pos = 0;
    str dst;
    str_from_c(&dst, "base_string_X");
    CU_ASSERT(str_last_of(&dst, 'X', &pos));
    CU_ASSERT(pos == 12);
    str_free(&dst);
}

void test_str_last_of_far(void) {
    size_t pos = 0;
    str dst;
    str_from_c(&dst, "X_base_string");
    CU_ASSERT(str_last_of(&dst, 'X', &pos));
    CU_ASSERT(pos == 0);
    str_free(&dst);
}

void test_str_equal(void) {
    str test_a;
    str test_b;

    str_from_c(&test_a, "prepended_testdataappended_X");
    str_from_c(&test_b, "prepended_testdataappended_X");
    CU_ASSERT(str_equal(&test_a, &test_b) == true);

    str_toupper(&test_a);
    CU_ASSERT(str_equal(&test_a, &test_b) == false);

    str_free(&test_a);
    str_free(&test_b);
}

void test_str_equal_c(void) {
    str d;
    str_from_c(&d, "prepended_testdataappended_X");
    CU_ASSERT(str_equal_c(&d, "prepended_testdataappended_X") == true);

    str_toupper(&d);
    CU_ASSERT(str_equal_c(&d, "prepended_testdataappended_X") == false);

    str_free(&d);
}

void test_str_equal_buf(void) {
    str d;
    str_from_c(&d, "xyz");

    CU_ASSERT(str_equal_buf(&d, "xyz", 3) == true);

    str_toupper(&d);
    CU_ASSERT(str_equal_buf(&d, "xyz", 3) == false);

    str_free(&d);
}

void test_str_replace_lg(void) {
    str d;
    str_from_c(&d, "test $1 string $2");
    str_replace(&d, "$1", "one", -1);
    str_replace(&d, "$2", "two", -1);
    CU_ASSERT(d.data[d.len] == 0);
    CU_ASSERT_STRING_EQUAL(d.data, "test one string two");
    str_free(&d);
}

void test_str_replace_eq(void) {
    str d;
    str_from_c(&d, "test $1 string $2");
    str_replace(&d, "$1", "11", -1);
    str_replace(&d, "$2", "22", -1);
    CU_ASSERT(d.data[d.len] == 0);
    CU_ASSERT_STRING_EQUAL(d.data, "test 11 string 22");
    str_free(&d);
}

void test_str_replace_sm(void) {
    str d;
    str_from_c(&d, "test $1 string $2");
    str_replace(&d, "$1", "1", -1);
    str_replace(&d, "$2", "2", -1);
    CU_ASSERT(d.data[d.len] == 0);
    CU_ASSERT_STRING_EQUAL(d.data, "test 1 string 2");
    str_free(&d);
}

void test_str_replace_multi(void) {
    str d;
    str_from_c(&d, "test $1 string $1");
    str_replace(&d, "$1", "one", -1);
    CU_ASSERT(d.data[d.len] == 0);
    CU_ASSERT_STRING_EQUAL(d.data, "test one string one");
    str_free(&d);
}

void test_str_replace_multi_limit(void) {
    str d;
    str_from_c(&d, "test $1 string $1");
    str_replace(&d, "$1", "one", 1);
    CU_ASSERT(d.data[d.len] == 0);
    CU_ASSERT_STRING_EQUAL(d.data, "test one string $1");
    str_free(&d);
}

void str_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "Test for str_create", test_str_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for string free operation", test_str_free) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_from", test_str_from) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_from_c", test_str_from_c) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_from_buf", test_str_from_buf) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_from_slice", test_str_from_slice) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_from_format", test_str_from_format) == NULL) {
        return;
    }

    if(CU_add_test(suite, "Test for str_toupper", test_str_toupper) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_tolower", test_str_tolower) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_rstrip", test_str_rstrip) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_lstrip", test_str_lstrip) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_strip", test_str_strip) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_append", test_str_append) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_append_c", test_str_append_c) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_append_buf", test_str_append_buf) == NULL) {
        return;
    }

    if(CU_add_test(suite, "Test for str_replace (longer replacement)", test_str_replace_lg) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_replace (equal replacement)", test_str_replace_eq) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_replace (shorter replacement)", test_str_replace_sm) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_replace (multiple hits)", test_str_replace_multi) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_replace (multiple hits w/limit)", test_str_replace_multi_limit) == NULL) {
        return;
    }

    if(CU_add_test(suite, "Test for str_first_of", test_str_first_of) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_last_of (near)", test_str_last_of_near) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_last_of (far)", test_str_last_of_far) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_equal", test_str_equal) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_equal_c", test_str_equal_c) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_equal_buf", test_str_equal_buf) == NULL) {
        return;
    }
}
