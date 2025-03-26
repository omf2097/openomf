#include "game/gui/text/text_layout.h"
#include "resources/fonts.h"
#include "utils/allocator.h"
#include "utils/str.h"
#include "video/surface.h"
#include <CUnit/CUnit.h>

void create_fake_font(font *font) {
    font_create(font);
    font->h = 8; // This is the default row height, if there is no text in it.
    font->size = FONT_BIG;

    // Create characters up to and including ' ' and 'A' as 8x8 pixels in size
    surface s;
    char end = 'A' - 32; // Fonts begin from index 32 in OMF font formats.
    for(int i = 0; i <= end; i++) {
        surface_create(&s, 8, 8);
        vector_append(&font->surfaces, &s);
    }

    // Create 'B' character 4x4 pixels in size, so that we can test different size combinations.
    surface_create(&s, 4, 4);
    vector_append(&font->surfaces, &s);
}

void test_find_next_line_end(void) {
    font f;
    str s;
    create_fake_font(&f);

    // First three letters should fit. there is no natural linebreak, so just cut.
    str_from_c(&s, "ABBABB");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_ROW_HORIZONTAL, 0, 0, 16, true), 3);
    str_free(&s);

    // Same as above, but in vertical direction,
    str_from_c(&s, "ABBABB");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_ROW_VERTICAL, 0, 0, 16, true), 3);
    str_free(&s);

    // No Letter spacing should make this just barely fit.
    str_from_c(&s, "AA");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_ROW_HORIZONTAL, 0, 0, 16, true), 2);
    str_free(&s);

    // One pixel letter spacing should make this not fit.
    str_from_c(&s, "AA");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_ROW_HORIZONTAL, 0, 1, 16, true), 1);
    str_free(&s);

    // Letter does not fit, so just return 0
    str_from_c(&s, "A");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_ROW_HORIZONTAL, 0, 0, 7, true), 0);
    str_free(&s);

    // More would fit, but we have a linebreak.
    str_from_c(&s, "A\nB");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_ROW_HORIZONTAL, 0, 0, 16, true), 2);
    str_free(&s);

    // We have a break point, but the whole line fits so it's not used.
    str_from_c(&s, "B B");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_ROW_HORIZONTAL, 0, 0, 16, true), 3);
    str_free(&s);

    // We have a break point (space), and the whole line doesn't fit so we must use it.
    str_from_c(&s, "A B");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_ROW_HORIZONTAL, 0, 0, 16, true), 2);
    str_free(&s);

    // We have a break point (dash), and the whole line doesn't fit so we must use it.
    str_from_c(&s, "A-B");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_ROW_HORIZONTAL, 0, 0, 16, true), 2);
    str_free(&s);

    // We start after space, and last character should fit on the last line.
    str_from_c(&s, "A BBBB");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_ROW_HORIZONTAL, 2, 0, 16, true), 6);
    str_free(&s);

    // Empty string should just return 0
    str_from_c(&s, "");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_ROW_HORIZONTAL, 0, 0, 16, true), 0);
    str_free(&s);

    font_free(&f);
}

void test_text_layout_compute(void) {
    font f;
    create_fake_font(&f);
    text_layout layout;
    text_layout_create(&layout);
    text_margin margin = {0, 0, 0, 0};

    str s;
    str_from_c(&s, "ABABB");

    text_layout_compute(&layout, &s, &f, 16, 24, TEXT_ALIGN_TOP, TEXT_ALIGN_LEFT, margin, TEXT_ROW_HORIZONTAL, 1, 0,
                        255);

    // Cleanup
    str_free(&s);
    text_layout_free(&layout);
    font_free(&f);
}

void text_layout_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for find_next_line_end", test_find_next_line_end) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for text_layout_compute", test_text_layout_compute) == NULL) {
        return;
    }
}
