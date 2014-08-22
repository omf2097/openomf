#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

// DEBUGMODE enables the leak detector
#ifndef DEBUGMODE
#define DEBUGMODE
#endif

#include <shadowdive/shadowdive.h>
#include "misc/parser_test_strings.h"

sd_stringparser *parser = NULL;

// --- Helper functions

void test_anim_string(const char *str) {
    int err = sd_stringparser_set_string(parser, str);
    if(err) {
        CU_FAIL("Parsing string failed.");
        printf("%s\n", str);
    } else {
        CU_ASSERT(sd_stringparser_run(parser, 0) == 0);
    }
}

void test_state_variables_str(const char *anim_str) {
    int err = sd_stringparser_set_string(parser, anim_str);
    CU_ASSERT(err == 0);
    if(err == 0) {
        const sd_stringparser_tag_value *blend_additive, *blend_start, *blend_end, *jump;
        int jump_count = 0;
        for(unsigned int tick = 0; tick < 2000; ++tick) {
            if(sd_stringparser_run(parser, tick) == 0) {
                sd_stringparser_frame *frame = &parser->current_frame;
                
                if(sd_stringparser_get_tag(parser, frame->id, SD_BLEND_ADDITIVE, &blend_additive)) {
                    CU_FAIL("Error getting blend additive tag");
                    return;
                }
                if(sd_stringparser_get_tag(parser, frame->id, SD_BLEND_START, &blend_start)) {
                    CU_FAIL("Error getting blend start tag");
                    return;
                }
                if(sd_stringparser_get_tag(parser, frame->id, SD_BLEND_FINISH, &blend_end)) {
                    CU_FAIL("Error getting blend end tag");
                    return;
                }
                if(sd_stringparser_get_tag(parser, frame->id, SD_JUMP_TICK, &jump)) {
                    CU_FAIL("Error getting jump tag");
                    return;
                }

                // limit jump count to avoid infinite loop
                if(jump_count > 3) break;
            }
        }
    } else {
        printf("%s (%s)\n", sd_get_error(err), anim_str);
        CU_FAIL("Animation string parser error");
    }
}

void find_leak() {
    sd_stringparser_mem *mem = sd_stringparser_mem_usage();
    for(int i = 0;i < sizeof(mem->allocs)/sizeof(sd_stringparser_alloc);i++){
        unsigned int alloced = mem->allocs[i].alloced;
        unsigned int freed = mem->allocs[i].freed;
        if(alloced > freed) {
            unsigned int leak = alloced - freed;
            printf("Leak detected at line %d, amount %u\n", mem->allocs[i].line, leak);
            CU_FAIL("Leak detected");
        } else if(alloced < freed) {
            unsigned int overfree = freed - alloced;
            printf("Overfree detected at line %d, amount %u\n", mem->allocs[i].line, overfree);
            CU_FAIL("Overfree detected");
        }
    }
}

/// --- Test functions

void test_sd_stringparser_create(void) {
    sd_stringparser_lib_init();
    parser = sd_stringparser_create();
    CU_ASSERT_PTR_NOT_NULL_FATAL(parser);
}

void test_sd_stringparser_delete(void) {
    sd_stringparser_delete(parser);
    sd_stringparser_lib_deinit();
}

void test_state_variables() {
    test_state_variables_str("brA20-bs200B200-bf200C200");
    test_state_variables_str("brA20-bs200B200-bf200C200-d1B10");
    find_leak();
}

void test_broken_string() {
    CU_ASSERT(sd_stringparser_set_string(parser, "x+4zcubs21l50zp") != 0);
}

void test_omf_strings() {
    for(int i = 0; i < TEST_STRING_COUNT; i++) {
        test_anim_string(test_strings[i]);
    }
}

void stringparser_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "test of test_sd_stringparser_create", test_sd_stringparser_create) == NULL) { return; }
    if(CU_add_test(suite, "test broken string", test_broken_string) == NULL) { return; }
    if(CU_add_test(suite, "test of all omf strings", test_omf_strings) == NULL) { return; }
    if(CU_add_test(suite, "test state variables", test_state_variables) == NULL) { return; }
    if(CU_add_test(suite, "test of test_sd_stringparser_delete", test_sd_stringparser_delete) == NULL) { return; }
}