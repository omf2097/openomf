#include "game/gui/text/text_layout.h"
#include "resources/fonts.h"
#include "utils/allocator.h"
#include "utils/str.h"
#include "video/surface.h"
#include <CUnit/CUnit.h>

void create_fake_font(font *font) {
    font_create(font);
    font->w = 8;
    font->h = 8;
    font->size = FONT_BIG;

    // Characters up to and including ' ' and 'A' are 8x8
    surface s;
    for(int i = 0; i <= 33; i++) {
        surface_create(&s, 8, 8);
        vector_append(&font->surfaces, &s);
    }

    // 'B' is 4x8
    surface_create(&s, 4, 8);
    vector_append(&font->surfaces, &s);
}

void test_find_next_line_end(void) {
    font f;
    str s;
    create_fake_font(&f);

    // First three letters should fit. there is no natural linebreak, so just cut.
    str_from_c(&s, "ABBABB");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_HORIZONTAL, 0, 16), 3);
    str_free(&s);

    // Letter does not fit, so just return 0
    str_from_c(&s, "A");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_HORIZONTAL, 0, 7), 0);
    str_free(&s);

    // More would fit, but we have a linebreak.
    str_from_c(&s, "A\nB");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_HORIZONTAL, 0, 16), 2);
    str_free(&s);

    // We have a linebreak, but the whole line fits so it's not used.
    str_from_c(&s, "B B");
    CU_ASSERT_EQUAL(find_next_line_end(&s, &f, TEXT_HORIZONTAL, 0, 16), 3);
    str_free(&s);

    font_free(&f);
}

void test_text_layout_compute(void) {
    font f;
    create_fake_font(&f);
    text_layout *layout = text_layout_create(16, 16);
    text_padding pad = {0, 0, 0, 0};

    str s;
    str_from_c(&s, "ABABB");

    fprintf(stderr, "\n");
    CU_ASSERT_EQUAL(text_layout_compute(layout, &s, &f, TEXT_TOP, TEXT_LEFT, pad, TEXT_HORIZONTAL, 255),
                    LAYOUT_NO_ERROR);

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
