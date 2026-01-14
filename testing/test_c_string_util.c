#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include <CUnit/CUnit.h>
#include <string.h>

void test_strncpy_or_truncate_normal(void) {
    char dest[20];
    const char *src = "Cthulhu fhtagn!";

    strncpy_or_truncate(dest, src, 20);
    CU_ASSERT_STRING_EQUAL(dest, "Cthulhu fhtagn!");
}

void test_strncpy_or_truncate_exact_fit(void) {
    char dest[16]; // exactly str + null
    const char *src = "Cthulhu fhtagn!";

    strncpy_or_truncate(dest, src, 16);
    CU_ASSERT_STRING_EQUAL(dest, "Cthulhu fhtagn!");
}

void test_strncpy_or_truncate_truncation(void) {
    char dest[8];
    const char *src = "Cthulhu fhtagn!";

    strncpy_or_truncate(dest, src, 8);
    CU_ASSERT_STRING_EQUAL(dest, "Cthulhu");
    CU_ASSERT_EQUAL(dest[7], '\0');
}

void test_strncpy_or_truncate_empty(void) {
    char dest[10];
    const char *src = "";

    strncpy_or_truncate(dest, src, 10);
    CU_ASSERT_STRING_EQUAL(dest, "");
}

void test_strncpy_or_truncate_size_one(void) {
    char dest[1];
    const char *src = "Cthulhu fhtagn!";

    strncpy_or_truncate(dest, src, 1);
    CU_ASSERT_EQUAL(dest[0], '\0');
}

void test_omf_strdup(void) {
    const char *original = "Cthulhu fhtagn!";
    char *duplicate = omf_strdup(original);

    CU_ASSERT_PTR_NOT_NULL(duplicate);
    CU_ASSERT_STRING_EQUAL(duplicate, original);
    CU_ASSERT_PTR_NOT_EQUAL(duplicate, original);

    omf_free(duplicate);
}

void test_omf_strcasecmp_equal(void) {
    CU_ASSERT_EQUAL(omf_strcasecmp("cthulhu fhtagn!", "cthulhu fhtagn!"), 0);
    CU_ASSERT_EQUAL(omf_strcasecmp("CTHULHU FHTAGN!", "cthulhu fhtagn!"), 0);
    CU_ASSERT_EQUAL(omf_strcasecmp("Cthulhu Fhtagn!", "cTHULHU fHTAGN!"), 0);
    CU_ASSERT_EQUAL(omf_strcasecmp("CtHuLhU fHtAgN!", "cthulhu fhtagn!"), 0);
}

void test_omf_strcasecmp_not_equal(void) {
    CU_ASSERT_NOT_EQUAL(omf_strcasecmp("abc", "abd"), 0);
    CU_ASSERT_NOT_EQUAL(omf_strcasecmp("abd", "abc"), 0);
    CU_ASSERT_NOT_EQUAL(omf_strcasecmp("ABC", "abd"), 0);
    CU_ASSERT_NOT_EQUAL(omf_strcasecmp("ABD", "abc"), 0);
    CU_ASSERT_NOT_EQUAL(omf_strcasecmp("abc", "abcd"), 0);
    CU_ASSERT_NOT_EQUAL(omf_strcasecmp("abcd", "abc"), 0);
}

void test_omf_strncasecmp_equal(void) {
    CU_ASSERT_EQUAL(omf_strncasecmp("cthulhu", "cthulhu", 7), 0);
    CU_ASSERT_EQUAL(omf_strncasecmp("CTHULHU", "cthulhu", 7), 0);
    CU_ASSERT_EQUAL(omf_strncasecmp("Cthulhu fhtagn!", "cthulhu rises!", 7), 0);
}

void test_omf_strncasecmp_not_equal(void) {
    CU_ASSERT_NOT_EQUAL(omf_strncasecmp("abc", "abd", 3), 0);
    CU_ASSERT_NOT_EQUAL(omf_strncasecmp("abd", "abc", 3), 0);
    CU_ASSERT_EQUAL(omf_strncasecmp("ABCXXX", "abcyyy", 3), 0);
    CU_ASSERT_NOT_EQUAL(omf_strncasecmp("ABCXXX", "abcyyy", 4), 0);
}

void test_omf_strnlen_s(void) {
    CU_ASSERT_EQUAL(omf_strnlen_s("Cthulhu", 100), 7);
    CU_ASSERT_EQUAL(omf_strnlen_s("Cthulhu fhtagn!", 100), 15);

    CU_ASSERT_EQUAL(omf_strnlen_s("Cthulhu fhtagn!", 7), 7);
    CU_ASSERT_EQUAL(omf_strnlen_s("Cthulhu fhtagn!", 10), 10);
}

void c_string_util_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "Test strncpy_or_truncate normal", test_strncpy_or_truncate_normal) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test strncpy_or_truncate exact fit", test_strncpy_or_truncate_exact_fit) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test strncpy_or_truncate truncation", test_strncpy_or_truncate_truncation) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test strncpy_or_truncate empty", test_strncpy_or_truncate_empty) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test strncpy_or_truncate size one", test_strncpy_or_truncate_size_one) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test omf_strdup", test_omf_strdup) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test omf_strcasecmp equal", test_omf_strcasecmp_equal) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test omf_strcasecmp not equal", test_omf_strcasecmp_not_equal) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test omf_strncasecmp equal", test_omf_strncasecmp_equal) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test omf_strncasecmp not equal", test_omf_strncasecmp_not_equal) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test omf_strnlen_s", test_omf_strnlen_s) == NULL) {
        return;
    }
}
