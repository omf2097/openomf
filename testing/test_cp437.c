#include "utils/cp437.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <uchar.h>

#include <locale.h>

extern char32_t const cp437_toutf32_lookup[];

// Make sure that the compiler is configured properly
static void test_source_charset(void) {
    // U+263A White Smiling Face
    CU_ASSERT_EQUAL(cp437_toutf32_lookup[0x01], 0x263A);
    // U+263B Black Smiling Face
    CU_ASSERT_EQUAL(cp437_toutf32_lookup[0x02], 0x263B);

    char const string[] = "☺☻";
    unsigned char const string2[] = {// U+263A White Smiling Face
                                     0xE2, 0x98, 0xBA,
                                     // U+263B Black Smiling Face
                                     0xE2, 0x98, 0xBB,
                                     // NUL byte
                                     0x00};
    static_assert((sizeof string) == (sizeof string2), "bad charset");
    CU_ASSERT(memcmp(string, string2, sizeof string) == 0);
}

static void test_cp437_utf32(void) {
    for(unsigned c = 0x00; c < 0x100; c++) {
        char32_t c32_groundtruth = cp437_toutf32_lookup[c];
        char32_t c32 = 0;
        uint8_t cp437_again;
        // shouldn't crash
        cp437_to_utf32(&c32, c);
        cp437_result from_utf32_result = cp437_from_utf32(&cp437_again, c32);
        int is_in_useless_range = 0xA9 <= c && c <= 0xDF;

        if(is_in_useless_range) {
            CU_ASSERT_EQUAL(from_utf32_result, CP437_ERROR_UNKNOWN_CODEPOINT);
        } else {
            CU_ASSERT_EQUAL(from_utf32_result, CP437_SUCCESS);
            CU_ASSERT_EQUAL(c, cp437_again);
        }

        if(c < 0x20) {
            // control characters pass through as-is
            CU_ASSERT_EQUAL(c32, (char32_t)c);
        } else if(!is_in_useless_range) {
            CU_ASSERT_EQUAL(c32, c32_groundtruth);
            CU_ASSERT_EQUAL(c, cp437_again);
        }

        // check CP437_MAX_UTF8_PER_CP437's invariant
        CU_ASSERT(cp437_toutf32_lookup[c] <= 0xFFFF);
    }
}

static void test_cp437_utf8_len(void) {
    // check that out len calculated with and without output buffer match
    for(unsigned c = 0x00; c < 0xFF; c++) {
        // cp437
        uint8_t buf_one[1] = {c};
        // utf-8
        unsigned char buf_two[CP437_MAX_UTF8_PER_CP437 * sizeof buf_one];

        size_t utf8_len = 99, utf8_len_withbuffer = 55;
        cp437_to_utf8(NULL, &utf8_len, buf_one, sizeof buf_one);
        CU_ASSERT_FATAL(utf8_len <= CP437_MAX_UTF8_PER_CP437 * sizeof buf_one);
        cp437_to_utf8(buf_two, &utf8_len_withbuffer, buf_one, sizeof buf_one);
        CU_ASSERT_EQUAL(utf8_len, utf8_len_withbuffer);

        // reverse conversion, too
        // cp437
        uint8_t buf_three[sizeof buf_two];
        size_t cp437_len, cp437_len_withbuffer;
        cp437_result err1 = cp437_from_utf8(NULL, &cp437_len, buf_two, utf8_len);
        cp437_result err2 = cp437_from_utf8(buf_three, &cp437_len_withbuffer, buf_two, utf8_len);
        CU_ASSERT_EQUAL(err1, err2);
        CU_ASSERT_EQUAL(cp437_len, cp437_len_withbuffer);
    }

    // reverse conversion, too
    // utf-8
    unsigned char unrecogn[] = u8"Robot 🤖";

// which behavior do I want?
#if 0
    // with NULL output buffer, cp437 doesn't check if codepoints map
    size_t cp437_len;
    cp437_result err1 = cp437_from_utf8(NULL, &cp437_len, unrecogn, sizeof unrecogn);
    CU_ASSERT_EQUAL(err1, CP437_SUCCESS);
    CU_ASSERT_EQUAL(8, cp437_len);
#else
    // with NULL output buffer, cp437 will still detect robot emoji
    // This behavior is good, because it makes the function behavior more consistent & easier to use
    size_t cp437_len;
    cp437_result err1 = cp437_from_utf8(NULL, &cp437_len, unrecogn, sizeof unrecogn);
    CU_ASSERT_EQUAL(err1, CP437_ERROR_UNKNOWN_CODEPOINT);
    CU_ASSERT_EQUAL(0, cp437_len);
#endif

    // cp437
    uint8_t unrecogn_cp437[sizeof unrecogn];
    size_t cp437_len_withbuffer;
    cp437_result err2 = cp437_from_utf8(unrecogn_cp437, &cp437_len_withbuffer, unrecogn, sizeof unrecogn);
    CU_ASSERT_EQUAL(err2, CP437_ERROR_UNKNOWN_CODEPOINT);
    CU_ASSERT_EQUAL(0, cp437_len_withbuffer);
}

static void test_cp437_utf8(void) {
    // ß (U+00DF) encodes in UTF-8 as 0xC3 0x9F
    unsigned char utf8[] = u8"Muß ich Dir ernsthaft erklären, was dies ist?";

    // calculate length
    size_t cp437_len;
    CU_ASSERT_EQUAL(cp437_from_utf8(NULL, &cp437_len, utf8, sizeof utf8), CP437_SUCCESS);
    // NOTE: this length includes the NUL byte, which we also passed into the conversion function
    uint8_t cp437[46];
    CU_ASSERT_EQUAL(cp437_len, sizeof cp437);

    // actually convert it
    uint8_t cp437_nolen[sizeof cp437];
    cp437_len = 0;
    CU_ASSERT_EQUAL(cp437_from_utf8(cp437, &cp437_len, utf8, sizeof utf8), CP437_SUCCESS);
    CU_ASSERT_EQUAL(cp437_len, sizeof cp437);
    CU_ASSERT_EQUAL(cp437_from_utf8(cp437_nolen, NULL, utf8, sizeof utf8), CP437_SUCCESS);
    CU_ASSERT_EQUAL(memcmp(cp437, cp437_nolen, sizeof cp437), 0);

    // convert it back
    size_t utf8_len;
    cp437_to_utf8(NULL, &utf8_len, cp437, cp437_len);
    CU_ASSERT_EQUAL(utf8_len, sizeof utf8);
    unsigned char utf8_again[sizeof utf8], utf8_again_nolen[sizeof utf8];
    utf8_len = 0;
    cp437_to_utf8(utf8_again, &utf8_len, cp437, cp437_len);
    CU_ASSERT_EQUAL(utf8_len, sizeof utf8);
    cp437_to_utf8(utf8_again_nolen, NULL, cp437, cp437_len);
    CU_ASSERT_EQUAL(memcmp(utf8_again, utf8_again_nolen, sizeof utf8), 0);
    CU_ASSERT_EQUAL(memcmp(utf8, utf8_again, sizeof utf8), 0);
}

void cp437_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "Test source-charset", test_source_charset) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test CP437 UTF-32 conversions", test_cp437_utf32) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test CP437 UTF-8 conversion string length", test_cp437_utf8_len) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test CP437 UTF-8 string conversions", test_cp437_utf8) == NULL) {
        return;
    }
}
