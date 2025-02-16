#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <assert.h>
#include <utils/str.h>

// The number of bytes (including null terminating byte) that can be stored in a small string.
#define STR_STACK_SIZE sizeof(str)
// detects whether a string is currently small-string-optimized
static bool is_small(str const *s) {
    void const *s_start = s;
    void const *s_end = s + 1;
    void const *s_str = str_c(s);
    // the string is small if the contents are stored within.
    return s_str >= s_start && s_str < s_end;
}

void test_str_create(void) {
    str m;
    str_create(&m);
    CU_ASSERT(str_size(&m) == 0);
    CU_ASSERT(is_small(&m));
    CU_ASSERT(m.small[0] == 0);
    str_free(&m);
}

void test_str_free(void) {
    str zeroed;
    memset(&zeroed, 0, sizeof zeroed);
    str m;
    str_create(&m);
    str_from_c(&m, "testdata");
    CU_ASSERT(memcmp(&m, &zeroed, sizeof(str)) != 0);
    str_free(&m);
    CU_ASSERT(memcmp(&m, &zeroed, sizeof(str)) == 0);
}

void test_str_from(void) {
    str src;
    str_from_c(&src, "testdata");

    str d;
    str_from(&d, &src);
    CU_ASSERT(str_size(&d) == 8);
    CU_ASSERT(is_small(&d));
    CU_ASSERT(str_c(&d)[7] == 'a');
    CU_ASSERT(str_c(&d)[8] == 0);

    str_free(&d);
    str_free(&src);
}

void test_str_from_threshold_short(void) {
    str d;
    char short_str[STR_STACK_SIZE];
    memset(short_str, 'A', sizeof short_str);
    short_str[sizeof short_str - 1] = '\0';
    str_from_c(&d, short_str);
    CU_ASSERT(is_small(&d));
    str_free(&d);
}

void test_str_from_threshold_long(void) {
    str d;
    char long_str[STR_STACK_SIZE + 1];
    memset(long_str, 'A', sizeof long_str);
    long_str[sizeof long_str - 1] = '\0';
    str_from_c(&d, long_str);
    CU_ASSERT(!is_small(&d));
    str_free(&d);
}

void test_str_from_long(void) {
    str src;
    str_from_c(&src, "testdatatestdatatestdatatestdata1"); // 33

    str d;
    str_from(&d, &src);
    CU_ASSERT(str_size(&d) == 33);
    CU_ASSERT(!is_small(&d));
    CU_ASSERT(str_c(&d)[32] == '1');
    CU_ASSERT(str_c(&d)[33] == 0);

    str_free(&d);
    str_free(&src);
}

void test_str_set_c(void) {
    str a;
    str_from_c(&a, "test");
    str_set_c(&a, "2222222");
    CU_ASSERT_STRING_EQUAL(a.small, "2222222");
    str_free(&a);
}

void test_str_set(void) {
    str a, b;
    str_from_c(&a, "test");
    str_from_c(&b, "2222222");

    str_set(&a, &b);
    CU_ASSERT_STRING_EQUAL(a.small, "2222222");

    str_free(&a);
    str_free(&b);
}

void test_str_from_c(void) {
    str d;
    str_from_c(&d, "testdata");
    CU_ASSERT(str_size(&d) == 8);
    CU_ASSERT(is_small(&d));
    CU_ASSERT(str_c(&d)[7] == 'a');
    CU_ASSERT(str_c(&d)[8] == 0);
    str_free(&d);
}

void test_str_from_c_long(void) {
    str d;
    str_from_c(&d, "testdatatestdatatestdatatestdata1");
    CU_ASSERT(str_size(&d) == 33);
    CU_ASSERT(!is_small(&d));
    CU_ASSERT(str_c(&d)[32] == '1');
    CU_ASSERT(str_c(&d)[33] == 0);
    str_free(&d);
}

void test_str_from_buf(void) {
    str d;
    str_from_buf(&d, "testdata", 8);
    CU_ASSERT(str_size(&d) == 8);
    CU_ASSERT(is_small(&d));
    CU_ASSERT(str_c(&d)[7] == 'a');
    CU_ASSERT(str_c(&d)[8] == 0);
    str_free(&d);
}

void test_str_from_buf_long(void) {
    str d;
    str_from_buf(&d, "testdatatestdatatestdatatestdata1", 33);
    CU_ASSERT(str_size(&d) == 33);
    CU_ASSERT(!is_small(&d));
    CU_ASSERT(str_c(&d)[32] == '1');
    CU_ASSERT(str_c(&d)[33] == 0);
    str_free(&d);
}

void test_str_from_format(void) {
    str d;
    str_from_format(&d, "%s, %d\n", "this is a test", 100);

    CU_ASSERT_STRING_EQUAL(str_c(&d), "this is a test, 100\n")
    CU_ASSERT(str_size(&d) == 20);
    CU_ASSERT(is_small(&d));
    CU_ASSERT(str_c(&d)[20] == 0);

    str_free(&d);
}

void test_str_from_format_long(void) {
    str d;
    str_from_format(&d, "%s, %d\n", "this is a test this is a test this is a test", 100);

    CU_ASSERT_STRING_EQUAL(str_c(&d), "this is a test this is a test this is a test, 100\n")
    CU_ASSERT(str_size(&d) == 50);
    CU_ASSERT(!is_small(&d));
    CU_ASSERT(str_c(&d)[50] == 0);

    str_free(&d);
}

void test_str_from_slice(void) {
    str src;
    str dst;

    str_from_c(&src, "test-data-a");
    str_from_slice(&dst, &src, 5, 5 + 4);

    CU_ASSERT(str_size(&dst) == 4);
    CU_ASSERT_NSTRING_EQUAL(str_c(&dst), "data", 5);

    str_free(&src);
    str_free(&dst);
}

void test_str_from_slice_long(void) {
    str src;
    str dst;

    str_from_c(&src, "test-data-a-test-data-a-test-data-a-test-data-a-test-data-a");
    str_from_slice(&dst, &src, 0, 33);

    CU_ASSERT(str_size(&dst) == 33);
    CU_ASSERT_NSTRING_EQUAL(str_c(&dst), "test-data-a-test-data-a-test-data", 33);

    str_free(&src);
    str_free(&dst);
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
    CU_ASSERT(str_c(&d)[str_size(&d)] == 0);

    str_free(&d);
}

void test_str_lstrip(void) {
    str d;
    str_from_c(&d, "  test  ");
    str_lstrip(&d);

    CU_ASSERT_NSTRING_EQUAL(str_c(&d), "test  ", 6);
    CU_ASSERT(str_c(&d)[str_size(&d)] == 0);

    str_free(&d);
}

void test_str_strip(void) {
    str d;
    str_from_c(&d, "  test  ");
    str_strip(&d);

    CU_ASSERT_NSTRING_EQUAL(str_c(&d), "test", 4);
    CU_ASSERT(str_c(&d)[str_size(&d)] == 0);

    str_free(&d);
}

void test_str_append(void) {
    str dst;
    str src;
    str_from_c(&src, "appended_string");
    str_from_c(&dst, "base_string");
    str_append(&dst, &src);
    CU_ASSERT(str_size(&dst) == 26);
    CU_ASSERT_PTR_NOT_NULL(str_c(&dst));
    CU_ASSERT_NSTRING_EQUAL(str_c(&dst), "base_stringappended_string", 27);
    CU_ASSERT(str_c(&dst)[str_size(&dst)] == '\0');
    str_free(&src);
    str_free(&dst);
}

void test_str_append_c(void) {
    str dst;
    str_from_c(&dst, "base_string");
    str_append_c(&dst, "appended_string");
    CU_ASSERT(str_size(&dst) == 26);
    CU_ASSERT_PTR_NOT_NULL(str_c(&dst));
    CU_ASSERT_NSTRING_EQUAL(str_c(&dst), "base_stringappended_string", 27);
    CU_ASSERT(str_c(&dst)[str_size(&dst)] == 0);
    str_free(&dst);
}

void test_str_append_buf(void) {
    str dst;
    str_from_c(&dst, "base_string");
    str_append_buf(&dst, "appended_string", 15);
    CU_ASSERT(str_size(&dst) == 26);
    CU_ASSERT_PTR_NOT_NULL(str_c(&dst));
    CU_ASSERT_NSTRING_EQUAL(str_c(&dst), "base_stringappended_string", 27);
    CU_ASSERT(str_c(&dst)[str_size(&dst)] == 0);
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
    str_from_c(&d, "TestData");
    CU_ASSERT(str_equal_c(&d, "TestData") == true);
    CU_ASSERT(str_equal_c(&d, "testdata") == false);
    CU_ASSERT(str_equal_c(&d, "Test") == false);
    CU_ASSERT(str_equal_c(&d, "TestDataData") == false);

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
    CU_ASSERT(str_c(&d)[str_size(&d)] == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "test one string two");
    str_free(&d);
}

void test_str_replace_eq(void) {
    str d;
    str_from_c(&d, "test $1 string $2");
    str_replace(&d, "$1", "11", -1);
    str_replace(&d, "$2", "22", -1);
    CU_ASSERT(str_c(&d)[str_size(&d)] == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "test 11 string 22");
    str_free(&d);
}

void test_str_replace_sm(void) {
    str d;
    str_from_c(&d, "test $1 string $2");
    str_replace(&d, "$1", "1", -1);
    str_replace(&d, "$2", "2", -1);
    CU_ASSERT(str_c(&d)[str_size(&d)] == 0);
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
    CU_ASSERT(!is_small(&d));

    str_replace(&d, "AA", "C", 1);
    CU_ASSERT(str_c(&d)[str_size(&d)] == '\0');
    CU_ASSERT(str_c(&d)[str_size(&d) - 1] == 'B');
    str_free(&d);
}

void test_str_replace_multi(void) {
    str d;
    str_from_c(&d, "test $1 string $1");
    str_replace(&d, "$1", "one", -1);
    CU_ASSERT(str_c(&d)[str_size(&d)] == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "test one string one");
    str_free(&d);
}

void test_str_replace_multi_regression(void) {
    str d;
    str_from_c(&d, "test $1 string");
    str_replace(&d, "$1", "$2 $1", 2);
    CU_ASSERT(str_c(&d)[str_size(&d)] == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "test $2 $1 string");
    str_free(&d);
}

void test_str_replace_multi_consecutive(void) {
    str d;
    str_from_c(&d, "test $1$1 string");
    str_replace(&d, "$1", "", -1);
    CU_ASSERT(str_c(&d)[str_size(&d)] == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "test  string");
    str_free(&d);
}

void test_str_replace_multi_limit(void) {
    str d;
    str_from_c(&d, "test $1 string $1");
    str_replace(&d, "$1", "one", 1);
    CU_ASSERT(str_c(&d)[str_size(&d)] == 0);
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
    CU_ASSERT(str_c(&d)[str_size(&d)] == '\0');
    CU_ASSERT(str_c(&d)[str_size(&d) - 1] == 'A');
    CU_ASSERT(str_c(&d)[0] == 'A');
    CU_ASSERT(str_delete_at(&d, STR_STACK_SIZE + 1) == false);
    for(size_t i = 0; i < STR_STACK_SIZE + 1; ++i) {
        CU_ASSERT(str_delete_at(&d, str_size(&d) - 1) == true);
    }
    CU_ASSERT(str_size(&d) == 0);
    CU_ASSERT(str_c(&d)[str_size(&d)] == '\0');
    CU_ASSERT(str_c(&d)[0] == '\0');

    str_free(&d);
}

void test_str_set_at(void) {
    str d;

    str_from_c(&d, "AAAA");

    CU_ASSERT(str_set_at(&d, 4, 'X') == false);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "AAAA");

    CU_ASSERT(str_set_at(&d, 3, 'X') == true);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "AAAX");
    CU_ASSERT(str_set_at(&d, 0, 'X') == true);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "XAAX");

    str_free(&d);
}

void test_str_truncate(void) {
    size_t initial_size = STR_STACK_SIZE + 1;
    str d;
    str_create(&d);
    for(size_t i = 0; i < initial_size; ++i) {
        str_append_c(&d, "A");
    }

    // These should do nothing.
    CU_ASSERT(str_size(&d) == initial_size);
    str_truncate(&d, initial_size * 2);
    CU_ASSERT(str_size(&d) == initial_size);
    str_truncate(&d, initial_size + 1);
    CU_ASSERT(str_size(&d) == initial_size);
    str_truncate(&d, initial_size);
    CU_ASSERT(str_size(&d) == initial_size);

    // These truncate.
    str_truncate(&d, STR_STACK_SIZE);
    CU_ASSERT(str_size(&d) == STR_STACK_SIZE);
    str_truncate(&d, 3);
    CU_ASSERT(str_size(&d) == 3);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "AAA");
    str_truncate(&d, 1);
    CU_ASSERT(str_size(&d) == 1);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "A");
    str_truncate(&d, 0);
    CU_ASSERT(str_size(&d) == 0);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "");

    str_free(&d);
}

void test_str_insert_at(void) {
    str d;

    str_from_c(&d, "ABCD");
    CU_ASSERT(str_insert_at(&d, 0, 'X') == true);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "XABCD");
    CU_ASSERT(str_insert_at(&d, 2, 'Y') == true);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "XAYBCD");
    CU_ASSERT(str_insert_at(&d, 6, 'Z') == true);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "XAYBCDZ");
    CU_ASSERT(str_insert_at(&d, 8, 'X') == false);

    size_t orig_size = 7;
    CU_ASSERT(str_size(&d) == orig_size);
    for(size_t i = 0; i < STR_STACK_SIZE; ++i) {
        CU_ASSERT(str_size(&d) == orig_size + i);
        CU_ASSERT(str_insert_at(&d, 4, 'W') == true);
        CU_ASSERT(str_size(&d) == orig_size + i + 1);
    }
    static_assert(STR_STACK_SIZE == 24, "following line needs to follow changes in STR_STACK_SIZE");
    CU_ASSERT_STRING_EQUAL(str_c(&d), "XAYBWWWWWWWWWWWWWWWWWWWWWWWWCDZ");

    str_free(&d);
}

void test_str_insert_c_at(void) {
    str d;

    str_from_c(&d, "ABCD");
    CU_ASSERT(str_insert_c_at(&d, 0, "XXX") == true);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "XXXABCD");
    CU_ASSERT(str_insert_c_at(&d, 4, "ZZZ") == true);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "XXXAZZZBCD");
    CU_ASSERT(str_insert_c_at(&d, 10, "YY") == true);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "XXXAZZZBCDYY");
    CU_ASSERT(str_insert_c_at(&d, 8, "UWUWUWUWUWUWUWUWU") == true);
    CU_ASSERT_STRING_EQUAL(str_c(&d), "XXXAZZZBUWUWUWUWUWUWUWUWUCDYY");

    str_free(&d);
}

void str_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "Test for str_create", test_str_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_free", test_str_free) == NULL) {
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
    if(CU_add_test(suite, "Test for str_set_at", test_str_set_at) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_set_c", test_str_set_c) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_set", test_str_set) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_truncate", test_str_truncate) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_insert_at", test_str_insert_at) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for str_insert_c_at", test_str_insert_c_at) == NULL) {
        return;
    }
}
