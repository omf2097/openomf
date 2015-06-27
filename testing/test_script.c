#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <shadowdive/shadowdive.h>
#include "misc/parser_test_strings.h"

sd_script script;

#define OK_STR "s05bpd1bps1bpn64A100-s1sf3B10-C34"
#define OK_STR_ENC "s5bpd1bps1bpn64A100-s1sf3B10-C34"

void test_script_create(void) {
    CU_ASSERT(sd_script_create(&script) == SD_SUCCESS);
    CU_ASSERT(sd_script_create(NULL) == SD_INVALID_INPUT);
    CU_ASSERT(script.frame_count == 0);
}

void test_script_decode(void) {
    CU_ASSERT(sd_script_decode(&script, NULL, NULL) == SD_INVALID_INPUT);
    CU_ASSERT(sd_script_decode(NULL, OK_STR, NULL) == SD_INVALID_INPUT);
    CU_ASSERT(sd_script_decode(&script, OK_STR, NULL) == SD_SUCCESS);
    CU_ASSERT(script.frame_count == 3);
    CU_ASSERT(script.frames[0].tag_count == 4);
    CU_ASSERT(script.frames[1].tag_count == 2);
    CU_ASSERT(script.frames[2].tag_count == 0);
}

void test_total_ticks(void) {
    CU_ASSERT(sd_script_get_total_ticks(&script) == 144);

    // New script should return 0
    sd_script m_script;
    sd_script_create(&m_script);
    CU_ASSERT(sd_script_get_total_ticks(&m_script) == 0);
    sd_script_free(&m_script);
}

void test_tick_pos_at_frame(void) {
    CU_ASSERT(sd_script_get_tick_pos_at_frame(&script, 0) == 0);
    CU_ASSERT(sd_script_get_tick_pos_at_frame(&script, 1) == 100);
    CU_ASSERT(sd_script_get_tick_pos_at_frame(&script, 2) == 110);
}

void test_tick_len_at_frame(void) {
    CU_ASSERT(sd_script_get_tick_len_at_frame(&script, 0) == 100);
    CU_ASSERT(sd_script_get_tick_len_at_frame(&script, 1) == 10);
    CU_ASSERT(sd_script_get_tick_len_at_frame(&script, 2) == 34);
}

void test_script_encoded_length(void) {
    int len = sd_script_encoded_length(&script);
    CU_ASSERT(len = strlen(OK_STR_ENC));
}

void test_script_encode(void) {
    char buf[1024];
    memset(buf, 0, 1024);
    CU_ASSERT(sd_script_encode(&script, buf) == SD_SUCCESS);
    CU_ASSERT(strcmp(OK_STR_ENC, buf) == 0);
}

void test_script_get_frame(void) {
    CU_ASSERT(sd_script_get_frame(&script, -1) == NULL);
    CU_ASSERT(sd_script_get_frame(&script, 0) != NULL);
    CU_ASSERT(sd_script_get_frame(&script, 1) != NULL);
    CU_ASSERT(sd_script_get_frame(&script, 2) != NULL);
    CU_ASSERT(sd_script_get_frame(&script, 3) == NULL);
}

void test_script_get_frame_at(void) {
    CU_ASSERT(sd_script_get_frame_at(&script, -1) == NULL);
    CU_ASSERT(sd_script_get_frame_at(&script, 0) == sd_script_get_frame(&script, 0));
    CU_ASSERT(sd_script_get_frame_at(&script, 99) == sd_script_get_frame(&script, 0));
    CU_ASSERT(sd_script_get_frame_at(&script, 100) == sd_script_get_frame(&script, 1));
    CU_ASSERT(sd_script_get_frame_at(&script, 109) == sd_script_get_frame(&script, 1));
    CU_ASSERT(sd_script_get_frame_at(&script, 110) == sd_script_get_frame(&script, 2));
    CU_ASSERT(sd_script_get_frame_at(&script, 143) == sd_script_get_frame(&script, 2));
    CU_ASSERT(sd_script_get_frame_at(&script, 144) == NULL);
}

void test_script_get_tag(void) {
    const sd_script_frame *frame = sd_script_get_frame(&script, 0);
    CU_ASSERT(sd_script_get_tag(frame, NULL) == NULL);
    CU_ASSERT(sd_script_get_tag(NULL, "a") == NULL);
    CU_ASSERT(sd_script_get_tag(frame, "bpd") != NULL);
    CU_ASSERT(sd_script_get_tag(frame, "bps") != NULL);
    CU_ASSERT(sd_script_get_tag(frame, "bpn") != NULL);
    CU_ASSERT(sd_script_get_tag(frame, "mp") == NULL);
    CU_ASSERT(sd_script_get_tag(frame, "forkingandcountry") == NULL);
}

void test_script_frame_changed(void) {
    CU_ASSERT(sd_script_frame_changed(&script, -1, 0) == 1);
    CU_ASSERT(sd_script_frame_changed(&script,  0, 0) == 0);
    CU_ASSERT(sd_script_frame_changed(&script,  0, 1) == 0);
    CU_ASSERT(sd_script_frame_changed(&script, 143, 144) == 1);
    CU_ASSERT(sd_script_frame_changed(&script, 99, 100) == 1);
    CU_ASSERT(sd_script_frame_changed(&script, 98, 99) == 0);
}

void test_script_get_frame_index(void) {
    CU_ASSERT(sd_script_get_frame_index(NULL, NULL) == -1);
    CU_ASSERT(sd_script_get_frame_index(&script, NULL) == -1);
    CU_ASSERT(sd_script_get_frame_index(&script, sd_script_get_frame(&script, -1)) == -1);
    CU_ASSERT(sd_script_get_frame_index(&script, sd_script_get_frame(&script, 0)) == 0);
    CU_ASSERT(sd_script_get_frame_index(&script, sd_script_get_frame(&script, 1)) == 1);
    CU_ASSERT(sd_script_get_frame_index(&script, sd_script_get_frame(&script, 2)) == 2);
    CU_ASSERT(sd_script_get_frame_index(&script, sd_script_get_frame(&script, 3)) == -1);
}

void test_script_get_frame_index_at(void) {
    CU_ASSERT(sd_script_get_frame_index_at(NULL, -1) == -1);
    CU_ASSERT(sd_script_get_frame_index_at(&script, -1) == -1);
    CU_ASSERT(sd_script_get_frame_index_at(&script, 0) == 0);
    CU_ASSERT(sd_script_get_frame_index_at(&script, 99) == 0);
    CU_ASSERT(sd_script_get_frame_index_at(&script, 100) == 1);
    CU_ASSERT(sd_script_get_frame_index_at(&script, 144) == -1);
}

void test_is_last_frame(void) {
    CU_ASSERT(sd_script_is_last_frame(NULL, NULL) == 0);
    CU_ASSERT(sd_script_is_last_frame(&script, sd_script_get_frame(&script, -1)) == 0);
    CU_ASSERT(sd_script_is_last_frame(&script, sd_script_get_frame(&script, 0)) == 0);
    CU_ASSERT(sd_script_is_last_frame(&script, sd_script_get_frame(&script, 2)) == 1);
    CU_ASSERT(sd_script_is_last_frame(&script, sd_script_get_frame(&script, 3)) == 0);
}

void test_is_last_frame_at(void) {
    CU_ASSERT(sd_script_is_last_frame_at(NULL, 0) == 0);
    CU_ASSERT(sd_script_is_last_frame_at(&script, -1) == 0);
    CU_ASSERT(sd_script_is_last_frame_at(&script, 0) == 0);
    CU_ASSERT(sd_script_is_last_frame_at(&script, 100) == 0);
    CU_ASSERT(sd_script_is_last_frame_at(&script, 110) == 1);
    CU_ASSERT(sd_script_is_last_frame_at(&script, 143) == 1);
    CU_ASSERT(sd_script_is_last_frame_at(&script, 144) == 0);
}

void test_is_first_frame(void) {
    CU_ASSERT(sd_script_is_first_frame(NULL, NULL) == 0);
    CU_ASSERT(sd_script_is_first_frame(&script, sd_script_get_frame(&script, -1)) == 0);
    CU_ASSERT(sd_script_is_first_frame(&script, sd_script_get_frame(&script, 0)) == 1);
    CU_ASSERT(sd_script_is_first_frame(&script, sd_script_get_frame(&script, 2)) == 0);
    CU_ASSERT(sd_script_is_first_frame(&script, sd_script_get_frame(&script, 3)) == 0);
}

void test_is_first_frame_at(void) {
    CU_ASSERT(sd_script_is_first_frame_at(NULL, 0) == 0);
    CU_ASSERT(sd_script_is_first_frame_at(&script, -1) == 0);
    CU_ASSERT(sd_script_is_first_frame_at(&script, 0) == 1);
    CU_ASSERT(sd_script_is_first_frame_at(&script, 99) == 1);
    CU_ASSERT(sd_script_is_first_frame_at(&script, 100) == 0);
    CU_ASSERT(sd_script_is_first_frame_at(&script, 110) == 0);
    CU_ASSERT(sd_script_is_first_frame_at(&script, 143) == 0);
    CU_ASSERT(sd_script_is_first_frame_at(&script, 144) == 0);
}

void test_script_isset(void) {
    CU_ASSERT(sd_script_isset(NULL, "bps") == 0);
    CU_ASSERT(sd_script_isset(sd_script_get_frame(&script, 0), "bps") == 1);
    CU_ASSERT(sd_script_isset(sd_script_get_frame(&script, 0), "bpd") == 1);
    CU_ASSERT(sd_script_isset(sd_script_get_frame(&script, 0), "mp") == 0);
}

void test_script_get(void) {
    CU_ASSERT(sd_script_get(NULL, "bps") == 0);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&script, 0), "bps") == 1);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&script, 0), "bpd") == 1);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&script, 0), "bpn") == 64);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&script, 0), "mp") == 0);
}

void test_script_tag_vars(void) {
    CU_ASSERT(sd_script_get(sd_script_get_frame(&script, 0), "s") == 5); // 05 -> 5 should work
}

void test_script_free(void) {
    sd_script_free(&script);
}

void test_script_all(void) {
    char buf[1024];
    for(int i = 0; i < TEST_STRING_COUNT; i++) {
        sd_script s;
        CU_ASSERT_FATAL(sd_script_create(&s) == SD_SUCCESS);
        int ret = sd_script_decode(&s, test_strings[i], NULL);
        if(ret == SD_SUCCESS) {
            CU_ASSERT(sd_script_encode(&s, buf) == SD_SUCCESS);
        } else {
            CU_FAIL("Parser failed. Broken string ?");
        }
        sd_script_free(&s);
    }
}

void test_next_frame_with_sprite(void) {
    CU_ASSERT(sd_script_next_frame_with_sprite(NULL, 0, 0) == -1); // script NULL
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, -1, 0) == -1); // nonexistent frame id
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 0, 1000) == -1); // Tick does not exist
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 2, 0) == 2); // C
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 1, 0) == 1); // B
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 0, 100) == -1); // A
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 3, 0) == -1); // D (does not exist)
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 0, 0) == -1); // A, exists but should not be found
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 0, -1) == 0); // A, test negative tick
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 0, 99) == -1); // A, exists but should not be found
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 1, 99) == 1); // B
}

void test_next_frame_with_tag(void) {
    CU_ASSERT(sd_script_next_frame_with_tag(NULL, "s", 0) == -1); // script NULL
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "xxx", 0) == -1); // nonexistent tag
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "s", 1000) == -1); // tick does not exist
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "s", 0) == 1); // Should be in frame 1
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "bpd", 0) == -1); // should be in frame 0, but should not be found
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "bpd", -1) == 0); // test negative tick
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "sf", 0) == 1); // Just test any tag
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "sf", 99) == 1); // Border case 1
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "sf", 100) == -1); // Border case 2
}

void test_set_tag(void) {
    // Test value setting to existing tag
    CU_ASSERT(sd_script_set_tag(&script, 0, "bpd", 10) == SD_SUCCESS);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&script, 0), "bpd") == 10);

    // Test creating new tag and make sure nothing got overwritten
    CU_ASSERT(sd_script_set_tag(&script, 1, "bpd", 50) == SD_SUCCESS);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&script, 1), "bpd") == 50);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&script, 1), "s") == 1);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&script, 1), "sf") == 3);

    // Check tag count
    CU_ASSERT(script.frames[1].tag_count == 3);

    // Bad input values
    CU_ASSERT(sd_script_set_tag(NULL, 1, "bpd", 50) == SD_INVALID_INPUT);
    CU_ASSERT(sd_script_set_tag(&script, -1, "bpd", 50) == SD_INVALID_INPUT);
    CU_ASSERT(sd_script_set_tag(&script, 1, "xxx", 50) == SD_INVALID_INPUT);
}

void test_letter_to_frame(void) {
    CU_ASSERT(sd_script_letter_to_frame('A') == 0);
    CU_ASSERT(sd_script_letter_to_frame('Z') == 25);
}

void test_frame_to_letter(void) {
    CU_ASSERT(sd_script_frame_to_letter(0) == 'A');
    CU_ASSERT(sd_script_frame_to_letter(25) == 'Z');
}

void script_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "test of sd_script_create", test_script_create) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_decode", test_script_decode) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_get_total_ticks", test_total_ticks) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_get_tick_pos_at_frame", test_tick_pos_at_frame) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_get_tick_len_at_frame", test_tick_len_at_frame) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_encoded_length", test_script_encoded_length) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_encode", test_script_encode) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_get_frame", test_script_get_frame) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_get_frame_at", test_script_get_frame_at) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_get_tag", test_script_get_tag) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_frame_changed", test_script_frame_changed) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_get_frame_index", test_script_get_frame_index) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_get_frame_index_at", test_script_get_frame_index_at) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_is_last_frame", test_is_last_frame) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_is_last_frame_at", test_is_last_frame_at) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_is_first_frame", test_is_first_frame) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_is_first_frame_at", test_is_first_frame_at) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_isset", test_script_isset) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_get", test_script_get) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_next_frame_with_sprite", test_next_frame_with_sprite) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_next_frame_with_tag", test_next_frame_with_tag) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_set_tag", test_set_tag) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_letter_to_frame", test_letter_to_frame) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_frame_to_letter", test_frame_to_letter) == NULL) { return; }
    if(CU_add_test(suite, "testing odd tags", test_script_tag_vars) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_free", test_script_free) == NULL) { return; }
    if(CU_add_test(suite, "test of all OMF strings", test_script_all) == NULL) { return; }
}
