#include "utils/path.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

void test_path_from_c(void) {
    path p;
    path_from_c(&p, "this/is/a/test");

    CU_ASSERT_STRING_EQUAL(p.buf, "this/is/a/test");
}

void test_path_from_parts(void) {
    path p;
    path_from_parts(&p, "this", "is", "a", "test");
    CU_ASSERT_STRING_EQUAL(p.buf, "this/is/a/test");

    path_from_parts(&p, "/", "this", "is", "a", "test");
    CU_ASSERT_STRING_EQUAL(p.buf, "/this/is/a/test");

    path_from_parts(&p, "C:\\", "this", "is", "a", "test");
    CU_ASSERT_STRING_EQUAL(p.buf, "C:/this/is/a/test");
}

void test_path_from_str(void) {
    str t;
    path p;
    str_from_c(&t, "this/is/a/test");
    path_from_str(&p, &t);
    str_free(&t);

    CU_ASSERT_STRING_EQUAL(p.buf, "this/is/a/test");
}

void path_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "test path_from_c", test_path_from_c) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test path_from_parts", test_path_from_parts) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test path_from_str", test_path_from_str) == NULL) {
        return;
    }
}
