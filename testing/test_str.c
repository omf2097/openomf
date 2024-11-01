#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <utils/str.h>

void test_str_create(void) {
    str m;
    str_create(&m);
    CU_ASSERT(m.len == 0);
    CU_ASSERT_PTR_NULL(m.data);
    CU_ASSERT(m.small[0] == 0);
    str_free(&m);
}

void test_str_from(void) {
    str src;
    str_from_c(&src, "testdata");

    str d;
    str_from(&d, &src);
    CU_ASSERT(str_size(&d) == 8);
    CU_ASSERT_PTR_NULL(d.data);
    CU_ASSERT(d.small[7] == 'a');
    CU_ASSERT(d.small[8] == 0);

    str_free(&d);
    str_free(&src);
}

void test_str_from_threshold_short(void) {
    str d;
    char short_str[STR_STACK_SIZE];
    memset(short_str, 'A', sizeof short_str);
    short_str[sizeof short_str - 1] = '\0';
    str_from_c(&d, short_str);
    CU_ASSERT_PTR_NULL(d.data);
    str_free(&d);
}

void test_str_from_threshold_long(void) {
    str d;
    char long_str[STR_STACK_SIZE + 1];
    memset(long_str, 'A', sizeof long_str);
    long_str[sizeof long_str - 1] = '\0';
    str_from_c(&d, long_str);
    CU_ASSERT_PTR_NOT_NULL(d.data);
    str_free(&d);
}

void test_str_from_long(void) {
    str src;
    str_from_c(&src, "testdatatestdatatestdatatestdata1"); // 33

    str d;
    str_from(&d, &src);
    CU_ASSERT(str_size(&d) == 33);
    CU_ASSERT_PTR_NOT_NULL(d.data);
    CU_ASSERT(d.data[32] == '1');
    CU_ASSERT(d.data[33] == 0);
    CU_ASSERT(d.small[0] == 0);

    str_free(&d);
    str_free(&src);
}

void test_str_from_c(void) {
    str d;
    str_from_c(&d, "testdata");
    CU_ASSERT(str_size(&d) == 8);
    CU_ASSERT_PTR_NULL(d.data);
    CU_ASSERT(d.small[7] == 'a');
    CU_ASSERT(d.small[8] == 0);
    str_free(&d);
}

void test_str_from_c_long(void) {
    str d;
    str_from_c(&d, "testdatatestdatatestdatatestdata1");
    CU_ASSERT(str_size(&d) == 33);
    CU_ASSERT_PTR_NOT_NULL(d.data);
    CU_ASSERT(d.data[32] == '1');
    CU_ASSERT(d.data[33] == 0);
    CU_ASSERT(d.small[0] == 0);
    str_free(&d);
}

void test_str_from_buf(void) {
    str d;
    str_from_buf(&d, "testdata", 8);
    CU_ASSERT(str_size(&d) == 8);
    CU_ASSERT_PTR_NULL(d.data);
    CU_ASSERT(d.small[7] == 'a');
    CU_ASSERT(d.small[8] == 0);
    str_free(&d);
}

void test_str_from_buf_long(void) {
    str d;
    str_from_buf(&d, "testdatatestdatatestdatatestdata1", 33);
    CU_ASSERT(str_size(&d) == 33);
    CU_ASSERT_PTR_NOT_NULL(d.data);
    CU_ASSERT(d.data[32] == '1');
    CU_ASSERT(d.data[33] == 0);
    CU_ASSERT(d.small[0] == 0);
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

void test_str_from_format_long(void) {
    str d;
    str_from_format(&d, "%s, %d\n", "this is a test this is a test this is a test", 100);

    CU_ASSERT_STRING_EQUAL(str_c(&d), "this is a test this is a test this is a test, 100\n")
    CU_ASSERT(d.len == 50);
    CU_ASSERT_PTR_NOT_NULL(d.data);
    CU_ASSERT(d.data[50] == 0);
    CU_ASSERT(d.small[0] == 0);

    str_free(&d);
}

void test_str_from_slice(void) {
    str src;
    str dst;

    str_from_c(&src, "test-data-a");
    str_from_slice(&dst, &src, 0, 4);

    CU_ASSERT(dst.len == 4);
    CU_ASSERT_NSTRING_EQUAL(dst.small, "test", 5);

    str_free(&src);
    str_free(&dst);
}

void test_str_from_slice_long(void) {
    str src;
    str dst;

    str_from_c(&src, "test-data-a-test-data-a-test-data-a-test-data-a-test-data-a");
    str_from_slice(&dst, &src, 0, 33);

    CU_ASSERT(dst.len == 33);
    CU_ASSERT_NSTRING_EQUAL(dst.data, "test-data-a-test-data-a-test-data", 33);

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
    str_from_c(&d, "test");
    str_toupper(&d);

    CU_ASSERT_NSTRING_EQUAL(str_c(&d), "TEST", 5);
    CU_ASSERT(str_c(&d)[5] == 0);

    str_free(&d);
}

void test_str_tolower(void) {
    str d;
    str_from_c(&d, "TEST");
    str_tolower(&d);

    CU_ASSERT_NSTRING_EQUAL(str_c(&d), "test", 5);
    CU_ASSERT(str_c(&d)[5] == 0);

    str_free(&d);
}

void test_str_rstrip(void) {
    str d;
    str_from_c(&d, "  test  ");
    str_rstrip(&d);

    CU_ASSERT_NSTRING_EQUAL(str_c(&d), "  test", 6);
    CU_ASSERT(str_c(&d)[d.len] == 0);

    str_free(&d);
}

void test_str_lstrip(void) {
    str d;
    str_from_c(&d, "  test  ");
    str_lstrip(&d);

    CU_ASSERT_NSTRING_EQUAL(str_c(&d), "test  ", 6);
    CU_ASSERT(str_c(&d)[d.len] == 0);

    str_free(&d);
}

void test_str_strip(void) {
    str d;
    str_from_c(&d, "  test  ");
    str_strip(&d);

    CU_ASSERT_NSTRING_EQUAL(d.small, "test", 4);
    CU_ASSERT(d.small[d.len] == 0);

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
    CU_ASSERT_NSTRING_EQUAL(dst.data, "base_stringappended_string", 27);
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
    CU_ASSERT_NSTRING_EQUAL(dst.data, "base_stringappended_string", 27);
    CU_ASSERT(dst.data[dst.len] == 0);
    str_free(&dst);
}

void test_str_append_buf(void) {
    str dst;
    str_from_c(&dst, "base_string");
    str_append_buf(&dst, "appended_string", 15);
    CU_ASSERT(str_size(&dst) == 26);
    CU_ASSERT_PTR_NOT_NULL(dst.data);
    CU_ASSERT_NSTRING_EQUAL(dst.data, "base_stringappended_string", 27);
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
    CU_ASSERT(str_c(&d)[d.len] == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "test one string two");
    str_free(&d);
}

void test_str_replace_eq(void) {
    str d;
    str_from_c(&d, "test $1 string $2");
    str_replace(&d, "$1", "11", -1);
    str_replace(&d, "$2", "22", -1);
    CU_ASSERT(str_c(&d)[d.len] == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "test 11 string 22");
    str_free(&d);
}

void test_str_replace_sm(void) {
    str d;
    str_from_c(&d, "test $1 string $2");
    str_replace(&d, "$1", "1", -1);
    str_replace(&d, "$2", "2", -1);
    CU_ASSERT(str_c(&d)[d.len] == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "test 1 string 2");
    str_free(&d);
}

void test_str_replace_sm_regression(void) {
    str d;
    char long_str[STR_STACK_SIZE + 1];
    memset(long_str, 'A', sizeof long_str);
    long_str[sizeof long_str - 2] = 'B';
    long_str[sizeof long_str - 1] = '\0';
    str_from_c(&d, long_str);
    CU_ASSERT_PTR_NOT_NULL(d.data);

    str_replace(&d, "AA", "C", 1);
    CU_ASSERT(str_c(&d)[d.len] == '\0');
    CU_ASSERT(str_c(&d)[d.len - 1] == 'B');
    str_free(&d);
}

void test_str_replace_multi(void) {
    str d;
    str_from_c(&d, "test $1 string $1");
    str_replace(&d, "$1", "one", -1);
    CU_ASSERT(str_c(&d)[d.len] == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "test one string one");
    str_free(&d);
}

void test_str_replace_multi_regression(void) {
    str d;
    str_from_c(&d, "test $1 string");
    str_replace(&d, "$1", "$2 $1", 2);
    CU_ASSERT(str_c(&d)[d.len] == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "test $2 $1 string");
    str_free(&d);
}

void test_str_replace_multi_consecutive(void) {
    str d;
    str_from_c(&d, "test $1$1 string");
    str_replace(&d, "$1", "", -1);
    CU_ASSERT(str_c(&d)[d.len] == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "test  string");
    str_free(&d);
}

void test_str_replace_multi_limit(void) {
    str d;
    str_from_c(&d, "test $1 string $1");
    str_replace(&d, "$1", "one", 1);
    CU_ASSERT(str_c(&d)[d.len] == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "test one string $1");
    str_free(&d);
}

void test_str_delete_at_small(void) {
    str d;

    str_from_c(&d, "ABCD");
    CU_ASSERT(str_size(&d) == 4);
    CU_ASSERT(str_delete_at(&d, 4) == false);
    CU_ASSERT(str_delete_at(&d, 0) == true);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "BCD");
    CU_ASSERT(str_size(&d) == 3);

    CU_ASSERT(str_delete_at(&d, 1) == true);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "BD");
    CU_ASSERT(str_size(&d) == 2);
    CU_ASSERT(str_delete_at(&d, 0) == true);
    CU_ASSERT(str_delete_at(&d, 0) == true);
    CU_ASSERT(str_delete_at(&d, 0) == false);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "");

    str_free(&d);
}

void test_str_delete_at_big(void) {
    str d;

    str_create(&d);
    for(size_t i = 0; i < STR_STACK_SIZE + 1; ++i) {
        str_append_c(&d, "A");
    }
    CU_ASSERT(str_size(&d) == STR_STACK_SIZE + 1);
    CU_ASSERT(str_c(&d)[d.len] == '\0');
    CU_ASSERT(str_c(&d)[d.len - 1] == 'A');
    CU_ASSERT(str_c(&d)[0] == 'A');
    CU_ASSERT(str_delete_at(&d, STR_STACK_SIZE + 1) == false);
    for(size_t i = 0; i < STR_STACK_SIZE + 1; ++i) {
        CU_ASSERT(str_delete_at(&d, d.len - 1) == true);
    }
    CU_ASSERT(str_size(&d) == 0);
    CU_ASSERT(str_c(&d)[d.len] == '\0');
    CU_ASSERT(str_c(&d)[0] == '\0');

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
    if(CU_add_test(suite, "Test for str_from small string threshold, short", test_str_from_threshold_short) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_from small string threshold, long", test_str_from_threshold_long) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for long str_from", test_str_from_long) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_from_c", test_str_from_c) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for long str_from_c", test_str_from_c_long) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_from_buf", test_str_from_buf) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for long str_from_buf", test_str_from_buf_long) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_from_slice", test_str_from_slice) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for long str_from_slice", test_str_from_slice_long) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_from_format", test_str_from_format) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for long str_from_format", test_str_from_format_long) == NULL) {
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
    if(CU_add_test(suite, "Test for str_replace (shorter replacement regression test)",
                   test_str_replace_sm_regression) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_replace (multiple hits)", test_str_replace_multi) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_replace (multiple hits, replacement contains seek)",
                   test_str_replace_multi_regression) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_replace (multiple consecutive hits w/empty)",
                   test_str_replace_multi_consecutive) == NULL) {
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
    if(CU_add_test(suite, "Test for small str_delete_at", test_str_delete_at_small) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for big str_delete_at", test_str_delete_at_big) == NULL) {
        return;
    }
}
