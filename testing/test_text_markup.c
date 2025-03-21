#include "game/gui/text/text.h"
#include "game/gui/text_render.h"
#include "resources/fonts.h"
#include "utils/allocator.h"
#include "utils/str.h"
#include "video/surface.h"
#include <CUnit/CUnit.h>

#define TEXT_SHADOW_GREEN 0xA2

static void create_fake_font(font *font, int w) {
    font_create(font);
    font->w = w;
    font->h = 8;
    font->size = FONT_BIG;

    surface s;
    for(int i = 0; i <= 128; i++) {
        surface_create(&s, w, 8);
        vector_append(&font->surfaces, &s);
    }
}

void test_text_generate_basic_document(void) {
    font f1, f2;
    text_document *doc = text_document_create();
    text_margin margin = {0, 0, 0, 0};

    create_fake_font(&f1, 8);
    create_fake_font(&f2, 6);

    fonts_set_font(&f1, FONT_BIG);
    fonts_set_font(&f2, FONT_SMALL);

    str s, s2;
    str_from_c(&s, "ABABB{SIZE 6}ababb{COLOR:YELLOW}yellow{COLOR:DEFAULT}default");

    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, ALIGN_TEXT_TOP,
                           ALIGN_TEXT_LEFT, margin, 1, 0, 0, 0);

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
    CU_ASSERT_EQUAL(text_get_color(t), COLOR_YELLOW);

    t = text_document_get_text(doc, 3);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "default"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_SMALL);
    CU_ASSERT_EQUAL(text_get_color(t), TEXT_BRIGHT_GREEN);

    // Cleanup
    str_free(&s);
    text_document_free(&doc);
    font_free(&f1);
    font_free(&f2);
}

void test_text_generate_multiline_document(void) {
    font f1, f2;
    text_document *doc = text_document_create();
    text_margin margin = {0, 0, 0, 0};

    create_fake_font(&f1, 8);
    create_fake_font(&f2, 6);

    fonts_set_font(&f1, FONT_BIG);
    fonts_set_font(&f2, FONT_SMALL);

    str s, s2;
    str_from_c(&s,
               "{WIDTH 260}{CENTER OFF}\n{SIZE 8}{SHADOWS ON}{COLOR:YELLOW}OpenOMF{COLOR:DEFAULT}\n\n{SIZE 6}{SPACING "
               "7}   Welcome to OpenOMF. If the thought of 50,000 lines of C code engineered to tear your sanity to "
               "shreds makes you cower in fear you'll love the Alt-F4 shortcut.\n\n{SIZE 6}{SPACING 9}{COLOR:YELLOW}My "
               "Hovercraft is full of eels!\n{SIZE 6}{SPACING 7}{COLOR:DEFAULT}   I'm sorry to hear that.");

    text_generate_document(doc, &s, FONT_BIG, 320, 200, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN, ALIGN_TEXT_TOP,
                           ALIGN_TEXT_LEFT, margin, 1, 0, 0, 0);

    CU_ASSERT_EQUAL(6, text_document_get_text_count(doc));

    text *t;

    t = text_document_get_text(doc, 0);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "\n"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_BIG);
    str_free(&s2);

    t = text_document_get_text(doc, 1);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "OpenOMF"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_BIG);
    CU_ASSERT_EQUAL(text_get_color(t), COLOR_YELLOW);
    str_free(&s2);

    t = text_document_get_text(doc, 2);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "\n\n"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_BIG);
    CU_ASSERT_EQUAL(text_get_color(t), TEXT_BRIGHT_GREEN);
    str_free(&s2);

    t = text_document_get_text(doc, 3);
    text_get_str(t, &s2);
    // printf("'%s'\n", str_c(&s2));
    CU_ASSERT(str_equal_c(&s2, "   Welcome to OpenOMF. If the thought of 50,000 lines of C code engineered to tear "
                               "your sanity to shreds makes you cower in fear you'll love the Alt-F4 shortcut.\n\n"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_SMALL);
    CU_ASSERT_EQUAL(text_get_color(t), TEXT_BRIGHT_GREEN);
    CU_ASSERT_EQUAL(text_get_line_spacing(t), 7);
    str_free(&s2);

    t = text_document_get_text(doc, 4);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "My Hovercraft is full of eels!\n"));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_SMALL);
    CU_ASSERT_EQUAL(text_get_color(t), COLOR_YELLOW);
    CU_ASSERT_EQUAL(text_get_line_spacing(t), 9);
    str_free(&s2);

    t = text_document_get_text(doc, 5);
    text_get_str(t, &s2);
    CU_ASSERT(str_equal_c(&s2, "   I'm sorry to hear that."));
    CU_ASSERT_EQUAL(text_get_font(t), FONT_SMALL);
    CU_ASSERT_EQUAL(text_get_color(t), TEXT_BRIGHT_GREEN);
    CU_ASSERT_EQUAL(text_get_line_spacing(t), 7);
    str_free(&s2);

    // Cleanup
    str_free(&s);
    text_document_free(&doc);
    font_free(&f1);
    font_free(&f2);
}

void text_markup_test_suite(CU_pSuite suite) {
    // Add tests
    if(CU_add_test(suite, "Test for text_generate_basic_document", test_text_generate_basic_document) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test for text_generate_multiline_document", test_text_generate_multiline_document) == NULL) {
        return;
    }
}
