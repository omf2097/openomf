#include "utils/fixedptc.h"
#include <CUnit/CUnit.h>

void test_zero_value(void) {
    char buf[FIXEDPT_STR_BUFSIZE];
    fixedpt zero = 0;
    size_t written;

    // Default precision (-1)
    written = fixedpt_str(zero, buf, sizeof(buf), -1);
    CU_ASSERT_STRING_EQUAL(buf, "0.00");
    CU_ASSERT_EQUAL(written, 4);

    // Trim all decimals (-3)
    written = fixedpt_str(zero, buf, sizeof(buf), -3);
    CU_ASSERT_STRING_EQUAL(buf, "0");
    CU_ASSERT_EQUAL(written, 1);
}

void test_positive_integers(void) {
    char buf[FIXEDPT_STR_BUFSIZE];
    fixedpt val = 123 << FIXEDPT_FBITS; // 123.0
    size_t written;

    // With 2 decimal places
    written = fixedpt_str(val, buf, sizeof(buf), 2);
    CU_ASSERT_STRING_EQUAL(buf, "123.00");
    CU_ASSERT_EQUAL(written, 6);

    // Trim trailing zeros
    written = fixedpt_str(val, buf, sizeof(buf), -3);
    CU_ASSERT_STRING_EQUAL(buf, "123");
    CU_ASSERT_EQUAL(written, 3);
}

void test_negative_numbers(void) {
    char buf[FIXEDPT_STR_BUFSIZE];
    fixedpt val = -(456 << FIXEDPT_FBITS); // -456.0
    size_t written;

    written = fixedpt_str(val, buf, sizeof(buf), 2);
    CU_ASSERT_STRING_EQUAL(buf, "-456.00");
    CU_ASSERT_EQUAL(written, 7);
}

void test_exact_fractions(void) {
    char buf[FIXEDPT_STR_BUFSIZE];
    fixedpt val = FIXEDPT_ONE + FIXEDPT_ONE_HALF; // 1.5
    size_t written;

    // Normal precision
    written = fixedpt_str(val, buf, sizeof(buf), 1);
    CU_ASSERT_STRING_EQUAL(buf, "1.5");
    CU_ASSERT_EQUAL(written, 3);

    // Trim zeros
    written = fixedpt_str(val, buf, sizeof(buf), -3);
    CU_ASSERT_STRING_EQUAL(buf, "1.5");
    CU_ASSERT_EQUAL(written, 3);
}

void test_buffer_limits(void) {
    char small_buf[5];
    fixedpt val = (123 << FIXEDPT_FBITS) + FIXEDPT_ONE_HALF; // 123.5
    size_t written;

    // Try to write to tiny buffer
    written = fixedpt_str(val, small_buf, sizeof(small_buf), 2);
    CU_ASSERT_STRING_EQUAL(small_buf, "123");
    CU_ASSERT_EQUAL(written, 3);
}

void test_zero_trimming(void) {
    char buf[FIXEDPT_STR_BUFSIZE];
    fixedpt val = (123 << FIXEDPT_FBITS) + (FIXEDPT_ONE / 4); // 123.25
    size_t written;

    // Add extra zeros
    written = fixedpt_str(val, buf, sizeof(buf), 4);
    CU_ASSERT_STRING_EQUAL(buf, "123.2500");
    CU_ASSERT_EQUAL(written, 8);

    // Trim to significant digits
    written = fixedpt_str(val, buf, sizeof(buf), -3);
    CU_ASSERT_STRING_EQUAL(buf, "123.25");
    CU_ASSERT_EQUAL(written, 6);
}

void test_max_precision(void) {
    char buf[FIXEDPT_STR_BUFSIZE];
    fixedpt val = FIXEDPT_ONE / 10; // 0.1
    size_t written;

    // Request maximum precision
    written = fixedpt_str(val, buf, sizeof(buf), -2);
    CU_ASSERT_STRING_EQUAL(buf, "0.097656250000000");
    CU_ASSERT_EQUAL(written, 17);
}

void test_min_value(void) {
    char buf[FIXEDPT_STR_BUFSIZE];
    fixedpt val = FIXEDPT_MIN; // INT32_MIN >> 8
    size_t written;

    written = fixedpt_str(val, buf, sizeof(buf), 2);
    CU_ASSERT_STRING_EQUAL(buf, "-8388608.00");
    CU_ASSERT_EQUAL(written, 11);
}

void test_near_min_value(void) {
    char buf[FIXEDPT_STR_BUFSIZE];
    fixedpt val = FIXEDPT_MIN + (FIXEDPT_ONE / 4);
    size_t written;

    written = fixedpt_str(val, buf, sizeof(buf), 2);
    CU_ASSERT_STRING_EQUAL(buf, "-8388607.75");
    CU_ASSERT_EQUAL(written, 11);
}

void test_max_value(void) {
    char buf[FIXEDPT_STR_BUFSIZE];
    fixedpt val = FIXEDPT_MAX; // INT32_MAX >> 8
    size_t written;

    written = fixedpt_str(val, buf, sizeof(buf), 2);
    CU_ASSERT_STRING_EQUAL(buf, "8388607.99");
    CU_ASSERT_EQUAL(written, 10);
}

void test_near_max_value(void) {
    char buf[FIXEDPT_STR_BUFSIZE];
    fixedpt val = FIXEDPT_MAX - (FIXEDPT_ONE / 4);
    size_t written;

    written = fixedpt_str(val, buf, sizeof(buf), 2);
    CU_ASSERT_STRING_EQUAL(buf, "8388607.74");
    CU_ASSERT_EQUAL(written, 10);
}

void fixedpt_str_test_suite(CU_pSuite suite) {
    CU_add_test(suite, "Zero Handling", test_zero_value);
    CU_add_test(suite, "Positive Integers", test_positive_integers);
    CU_add_test(suite, "Negative Numbers", test_negative_numbers);
    CU_add_test(suite, "Exact Fractions", test_exact_fractions);
    CU_add_test(suite, "Buffer Limits", test_buffer_limits);
    CU_add_test(suite, "Zero Trimming", test_zero_trimming);
    CU_add_test(suite, "Max Precision", test_max_precision);
    CU_add_test(suite, "Min Value", test_min_value);
    CU_add_test(suite, "Near Min Value", test_near_min_value);
    CU_add_test(suite, "Max Value", test_max_value);
    CU_add_test(suite, "Near Max Value", test_near_max_value);
}
