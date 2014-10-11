#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <game/gui/text_render.h>

void test_text_find_max_strlen(void) {
    CU_ASSERT(text_find_max_strlen(10, "          AAAAAAAAAA") == 20); // Space should be disregarded and only 10 accepted. Advance should be 20.
    CU_ASSERT(text_find_max_strlen(10, "AAAAAAAAAA") == 10); // This should go on one line
    CU_ASSERT(text_find_max_strlen(10, "AAAAAAAAAAA") == 11); // This should go on one line, too, even though it's too long
    CU_ASSERT(text_find_max_strlen(10, "AAAAAAAAAA BBBB") == 10); // Only first string should be taken into account (10). Space should be disregarded.
    CU_ASSERT(text_find_max_strlen(10, "AAAAAAAAAAA BBBB") == 11); // Only first string should be taken into account (11)
    CU_ASSERT(text_find_max_strlen(10, "AA\n") == 3); // Should advance 2 chars + line ending (3)
    CU_ASSERT(text_find_max_strlen(14, "I CAN HAS CHEESEBURGER") == 10); // Should wordwrap. (10)
}

void test_text_find_line_count(void) {
    CU_ASSERT(text_find_line_count(TEXT_HORIZONTAL, 5, 5, strlen("AAAAAAAAA"), "AAAAAAAAA") == 1);
    CU_ASSERT(text_find_line_count(TEXT_HORIZONTAL, 5, 5, 11, "AAA AAA AAA") == 3);
}

void text_render_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for text_find_max_strlen", test_text_find_max_strlen) == NULL) { return; }
    if(CU_add_test(suite, "Test for text_find_line_count", test_text_find_line_count) == NULL) { return; }
}
