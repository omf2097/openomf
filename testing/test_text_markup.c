#include "game/gui/text/text.h"
#include "game/gui/text_render.h"
#include "resources/fonts.h"
#include "utils/allocator.h"
#include "utils/str.h"
#include "video/surface.h"
#include <CUnit/CUnit.h>

#define TEXT_SHADOW_GREEN 0xA2

static void create_fake_font(font *font, int h) {
    font_create(font);
    font->w = 8;
    font->h = h;
    font->size = FONT_BIG;

    surface s;
    for(int i = 0; i <= 128; i++) {
        surface_create(&s, 8, h);
        vector_append(&font->surfaces, &s);
    }
}

void test_text_generate_basic_document(void) {
    text_document *doc = text_document_create();
    text_margin margin = {0, 0, 0, 0};

    str s, s2;
    str_from_c(&s, "ABABB{SIZE 6}ababb{COLOR:YELLOW}yellow{COLOR:DEFAULT}default");

    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, TEXT_ALIGN_TOP,
                           TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);

    CU_ASSERT_EQUAL(4, text_document_get_text_count(doc));

    text *t;

    t = text_document_get_text(doc, 0);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "ABABB"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_BIG);

    t = text_document_get_text(doc, 1);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "ababb"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_SMALL);

    t = text_document_get_text(doc, 2);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "yellow"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_SMALL);
    CU_ASSERT_EQUAL(text_get_color(t), TEXT_YELLOW);

    t = text_document_get_text(doc, 3);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "default"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_SMALL);
    CU_ASSERT_EQUAL(text_get_color(t), TEXT_BRIGHT_GREEN);

    // Cleanup
    str_free(&s);
    text_document_free(&doc);
}

void test_text_generate_multiline_document(void) {
    text_document *doc = text_document_create();
    text_margin margin = {0, 0, 0, 0};

    str s, s2;
    str_from_c(&s,
               "{WIDTH 260}{CENTER OFF}\n{SIZE 8}{SHADOWS ON}{COLOR:YELLOW}OpenOMF{COLOR:DEFAULT}\n\n{SIZE 6}{SPACING "
               "7}   Welcome to OpenOMF. If the thought of 50,000 lines of C code engineered to tear your sanity to "
               "shreds makes you cower in fear you'll love the Alt-F4 shortcut.\n\n{SIZE 6}{SPACING 9}{COLOR:YELLOW}My "
               "Hovercraft is full of eels!\n{SIZE 6}{SPACING 7}{COLOR:DEFAULT}   I'm sorry to hear that.");

    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, TEXT_ALIGN_TOP,
                           TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);

    CU_ASSERT_EQUAL(5, text_document_get_text_count(doc));

    text *t;

    t = text_document_get_text(doc, 0);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "OpenOMF"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_BIG);
    CU_ASSERT_EQUAL(text_get_color(t), TEXT_YELLOW);
    str_free(&s2);

    t = text_document_get_text(doc, 1);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "\n"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_BIG);
    CU_ASSERT_EQUAL(text_get_color(t), TEXT_BRIGHT_GREEN);
    str_free(&s2);

    t = text_document_get_text(doc, 2);
    text_get_str(t, &s2);
    // printf("'%s'\n", str_c(&s2));
    CU_ASSERT(str_equal_c(&s2, "   Welcome to OpenOMF. If the thought of 50,000 lines of C code engineered to tear "
                               "your sanity to shreds makes you cower in fear you'll love the Alt-F4 shortcut.\n\n"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_SMALL);
    CU_ASSERT_EQUAL(text_get_color(t), TEXT_BRIGHT_GREEN);
    CU_ASSERT_EQUAL(text_get_line_spacing(t), 1); // spacing 7 - size 6
    str_free(&s2);

    t = text_document_get_text(doc, 3);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "My Hovercraft is full of eels!\n"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_SMALL);
    CU_ASSERT_EQUAL(text_get_color(t), TEXT_YELLOW);
    CU_ASSERT_EQUAL(text_get_line_spacing(t), 3); // spacing 9 - size 6
    str_free(&s2);

    t = text_document_get_text(doc, 4);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "   I'm sorry to hear that."));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_SMALL);
    CU_ASSERT_EQUAL(text_get_color(t), TEXT_BRIGHT_GREEN);
    CU_ASSERT_EQUAL(text_get_line_spacing(t), 1);
    str_free(&s2);

    // Cleanup
    str_free(&s);
    text_document_free(&doc);
}

void test_empty_and_pure_markup(void) {
    text_document *doc = text_document_create();
    text_margin margin = {0};
    str s;

    // Test 1a: Empty input
    str_from_c(&s, "");
    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, TEXT_ALIGN_TOP,
                           TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);
    CU_ASSERT_EQUAL(0, text_document_get_text_count(doc));

    // Test 1b: Only markup, no text
    str_from_c(&s, "{SIZE 8}{COLOR 255}{SHADOWS OFF}");
    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, TEXT_ALIGN_TOP,
                           TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);
    CU_ASSERT_EQUAL(0, text_document_get_text_count(doc)); // Empty text object?

    text_document_free(&doc);
    str_free(&s);
}

void test_numeric_edge_cases(void) {
    text_document *doc = text_document_create();
    text_margin margin = {0};
    str s;

    // Test 2a: Width exceeding max uint16_t (65535)
    str_from_c(&s, "{WIDTH 65536}Text");
    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, TEXT_ALIGN_TOP,
                           TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);
    // Verify width clamping or error handling

    // Test 2b: Negative spacing
    str_from_c(&s, "{SPACING -5}Text");
    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, TEXT_ALIGN_TOP,
                           TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);
    // Verify uint8_t underflow handling

    text_document_free(&doc);
    str_free(&s);
}

void test_tag_overlap(void) {
    text_document *doc = text_document_create();
    text_margin margin = {0};
    str s;

    // Test 3a: Font size cascading changes
    str_from_c(&s, "{SIZE 8}Big{SIZE 6}Small{SIZE 8}BigAgain");
    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, TEXT_ALIGN_TOP,
                           TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);

    CU_ASSERT_EQUAL(3, text_document_get_text_count(doc));
    text *t = text_document_get_text(doc, 0);
    CU_ASSERT_EQUAL(text_get_font(t), FONT_BIG);

    t = text_document_get_text(doc, 1);
    CU_ASSERT_EQUAL(text_get_font(t), FONT_SMALL);

    t = text_document_get_text(doc, 2);
    CU_ASSERT_EQUAL(text_get_font(t), FONT_BIG); // Verify font switches back

    text_document_free(&doc);
    str_free(&s);
}

void test_special_characters(void) {
    text_document *doc = text_document_create();
    text_margin margin = {0};
    str s, s2;

    // Test 4a: Braces in text that aren't markup
    str_from_c(&s, "Text{with}fake{markup");
    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, TEXT_ALIGN_TOP,
                           TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);

    text *t = text_document_get_text(doc, 0);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "Text"));

    t = text_document_get_text(doc, 1);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "fake"));

    text_document_free(&doc);
    str_free(&s);
}

void test_broken_markup(void) {
    text_document *doc = text_document_create();
    text_margin margin = {0};
    str s, s2;

    // Test 5a: Unterminated {WIDTH
    str_from_c(&s, "{WIDTH 100 TextWithoutClosingBrace");
    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, TEXT_ALIGN_TOP,
                           TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);

    // whole document is empty
    CU_ASSERT_EQUAL(0, text_document_get_text_count(doc));
    str_free(&s);

    // Test 5b: Garbage after valid markup
    str_from_c(&s, "{SIZE 8}ABC}{SHADOWS ON");
    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, TEXT_ALIGN_TOP,
                           TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);
    CU_ASSERT_EQUAL(1, text_document_get_text_count(doc));
    text *t = text_document_get_text(doc, 0);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "ABC}"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_BIG);

    text_document_free(&doc);
    str_free(&s);
}

void test_color_formats(void) {
    text_document *doc = text_document_create();
    text_margin margin = {0};
    str s;

    str_from_c(&s, "{COLOR:YELLOW}Named{COLOR 200}Numeric");
    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, TEXT_ALIGN_TOP,
                           TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);

    text *t = text_document_get_text(doc, 0);
    CU_ASSERT_EQUAL(text_get_color(t), TEXT_YELLOW);

    t = text_document_get_text(doc, 1);
    CU_ASSERT_EQUAL(text_get_color(t), 200); // Verify numeric format

    text_document_free(&doc);
    str_free(&s);
}

void test_font_family_switching(void) {
    // Verify SIZE commands respect CURRENT font, not initial
    text_document *doc = text_document_create();
    text_margin margin = {0};
    str s;

    str_from_c(&s, "{SIZE 8}Big{SIZE 6}Small1{SIZE 6}Small2");
    text_generate_document(doc, &s, FONT_NET1, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, TEXT_ALIGN_TOP,
                           TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);

    text *t = text_document_get_text(doc, 0);
    CU_ASSERT_EQUAL(text_get_font(t), FONT_NET1);

    t = text_document_get_text(doc, 1);
    CU_ASSERT_EQUAL(text_get_font(t), FONT_NET2); // Initial was NET1

    t = text_document_get_text(doc, 2);
    CU_ASSERT_EQUAL(text_get_font(t), FONT_NET2); // Still NET2

    text_document_free(&doc);
    str_free(&s);
}

int text_markup_suite_init(void) {
    font f1, f2, f3, f4;
    create_fake_font(&f1, 8);
    create_fake_font(&f2, 6);

    fonts_set_font(&f1, FONT_BIG);
    fonts_set_font(&f2, FONT_SMALL);

    create_fake_font(&f3, 8);
    create_fake_font(&f4, 6);

    fonts_set_font(&f3, FONT_NET1);
    fonts_set_font(&f4, FONT_NET2);

    return 0;
}

int text_markup_suite_free(void) {
    font_free((font *)fonts_get_font(FONT_SMALL));
    font_free((font *)fonts_get_font(FONT_BIG));
    font_free((font *)fonts_get_font(FONT_NET1));
    font_free((font *)fonts_get_font(FONT_NET2));
    return 0;
}

void text_markup_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for text_generate_basic_document", test_text_generate_basic_document) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for text_generate_multiline_document", test_text_generate_multiline_document) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Empty/Pure Markup", test_empty_and_pure_markup) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Numeric Edge Cases", test_numeric_edge_cases) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Tag Overlap", test_tag_overlap) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Special Chars", test_special_characters) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Broken Markup", test_broken_markup) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Color Formats", test_color_formats) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Font Switching", test_font_family_switching) == NULL) {
        return;
    }
}
