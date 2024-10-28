#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <game/gui/text_render.h>

void test_text_find_max_strlen(void) {
    text_settings tconf;
    text_defaults(&tconf);
    tconf.strip_leading_whitespace = true;
    // Space should be disregarded. Advance should be 10 spaces + 10 A's.
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 10, "          AAAAAAAAAA"), 20);
    // Space should be disregarded. Advance should be 20 spaces + 10 A's.
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 10, "                    AAAAAAAAAA"), 30);
    tconf.strip_leading_whitespace = false;
    // Space should NOT be disregarded and all accepted.
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 10, "          AAAAAAAAAA"), 10);
    // Space should NOT be disregarded and all accepted.
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 10, "        AAAAAAAAAA"), 8);
    // Space should NOT be disregarded and only 10 accepted. Advance should be 10.
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 10, "                    AAAAAAAAAA"), 10);
    // This should go on one line
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 10, "AAAAAAAAAA"), 10);
    // This should not go on one line, it's too long
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 10, "AAAAAAAAAAA"), 10);
    // Only first string should be taken into account
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 10, "AAAAAAA BBBBB"), 8);
    // Should advance 2 chars + line ending
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 10, "AA\n"), 3);
    // Should wordwrap, advancing 9 chars + space
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 14, "I CAN HAS CHEESEBURGER"), 10);
}

// to simplify tests, pass in full string length
static int text_find_line_count_(const text_settings *settings, int cols, int rows, const char *text, int *longest) {
    return text_find_line_count(settings, cols, rows, text, longest);
}

void test_text_find_line_count(void) {
    text_settings tconf;
    text_defaults(&tconf);
    int longest = 0;
    tconf.strip_trailing_whitespace = false;
    // word is broken in half because there are no spaces
    CU_ASSERT(text_find_line_count(&tconf, 5, 5, "AAAAAAAAA", &longest) == 1);
    CU_ASSERT_EQUAL(5, longest);
    CU_ASSERT(text_find_line_count(&tconf, 5, 5, "AAA AAA AAA", &longest) == 3);
    CU_ASSERT_EQUAL(4, longest);
}

static void test_text_find_max_strlen_newsroom(void) {
    text_settings tconf;
    text_defaults(&tconf);
    tconf.strip_leading_whitespace = false;
    tconf.strip_trailing_whitespace = true;
    tconf.max_lines = 9;

    /*
    Normal OMF newsroom layout
    (note: they strip the trailing spaces after word-wrap, before centering)

    "Whoa, this challenger meant "
    "business tonight.  Shirro "
    "could show the old pros a "
    "thing or two about that Jaguar."
    */

    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 31, "Whoa, this challenger meant business "), 28);
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 31, "business tonight.  Shirro could "), 26);
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 31, "could show the old pros a thing "), 26);
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 31, "thing or two about that Jaguar."), 31);

    // adding a trailing space causes "Jaguar. " to linewrap
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 31, "thing or two about that Jaguar. "), 24);
    // multiple trailing spaces are all part of "Jaguar  " word
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 31, "thing or two about that Jaguar  "), 24);
    // leading spaces are not stripped
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 31, "    Whoa, this challenger meant"), 31);
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 31, "     Whoa, this challenger meant"), 27);

    // even without spaces, words break.
    CU_ASSERT_EQUAL(text_find_max_strlen(&tconf, 31, "WhoaXXthisXchallengerXmeantXbusinessXtonight"), 31);
}

static void test_text_find_line_count_newsroom(void) {
    text_settings tconf;
    text_defaults(&tconf);
    tconf.strip_leading_whitespace = false;
    tconf.strip_trailing_whitespace = true;
    tconf.max_lines = 9;
    int longest = 0;

    // trailing spaces are counted as part of the previous word
    // clang-format off
    CU_ASSERT_EQUAL(text_find_line_count_(&tconf, 31, 999,
                                          "Whoa, this challenger meant "
                                          "business tonight.  Shirro "
                                          "could show the old pros a "
                                          "thing or two about that Jaguar.",
                                          &longest),
                    4);
    CU_ASSERT_EQUAL(31, longest);
    CU_ASSERT_EQUAL(text_find_line_count_(&tconf, 31, 999,
                                          "Whoa, this challenger meant "
                                          "business tonight.  Shirro "
                                          "could show the old pros a "
                                          "thing or two about that "
                                          "Jaguar. ",
                                          &longest),
                    5);
    CU_ASSERT_EQUAL(28, longest);

    CU_ASSERT_EQUAL(text_find_line_count_(&tconf, 31, 999,
                                          "Whoa, this challenger meant "
                                          "business tonight.  Shirro "
                                          "could show the old pros a "
                                          "thing or two about that "
                                          "Jaguar  ",
                                          &longest),
                    5);
    CU_ASSERT_EQUAL(28, longest);
    //clang-format on

    // leading spaces are not stripped
    CU_ASSERT_EQUAL(text_find_line_count_(&tconf, 31, 999, "    Whoa, this challenger meant", &longest), 1);
    CU_ASSERT_EQUAL(31, longest);

    CU_ASSERT_EQUAL(text_find_line_count_(&tconf, 31, 999, "     Whoa, this challenger meant", &longest), 2);
    CU_ASSERT_EQUAL(27, longest);

    // even without spaces, words break.
    CU_ASSERT_EQUAL(text_find_line_count_(&tconf, 31, 999, "WhoaXXthisXchallengerXmeantXbusinessXtonight", &longest),
                    2);
    CU_ASSERT_EQUAL(31, longest);

    // linefeeds are ignored after reaching max_lines
    // note: displays as nineteneleventwelve, but unsure how to test for that.
    CU_ASSERT_EQUAL(text_find_line_count_(&tconf, 31, 999,
                                          "one\ntwo\nthree\nfour\nfive\nsix\nseven\neight\nnine\nten\neleven\ntwelve",
                                          &longest),
                    9);
    CU_ASSERT_EQUAL(6, longest);

    // after reaching max_lines, word wrap stops (and no more words are rendered, but we don't care about that behavior)
    CU_ASSERT_EQUAL(text_find_line_count_(&tconf, 2, 999,
                                          "one two three four five six seven eight nine ten eleven twelve", &longest),
                    9);
    CU_ASSERT_EQUAL(2, longest);
}

static void test_text_find_max_strlen_pilot_bio(void) {
    text_settings tconf;
    text_defaults(&tconf);
    tconf.strip_leading_whitespace = false;
    tconf.strip_trailing_whitespace = false;
    tconf.max_lines = 8;
    CU_ASSERT_EQUAL(
        text_find_max_strlen(&tconf, 26, "her reclusive disposition and strong will, little is known of her."), 26);
}

void text_render_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for text_find_max_strlen", test_text_find_max_strlen) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for text_find_line_count", test_text_find_line_count) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for text_find_max_strlen with newsroom quirks", test_text_find_max_strlen_newsroom) ==
       NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for text_find_line_count with newsroom quirks", test_text_find_line_count_newsroom) ==
       NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for text_find_max_strlen with pilot bio quirks", test_text_find_max_strlen_pilot_bio) ==
       NULL) {
        return;
    }
}
