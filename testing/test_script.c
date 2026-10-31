#include "common.h"
#include "formats/error.h"
#include "formats/script.h"
#include "formats/tag_list_helpers.h"
#include "misc/parser_test_strings.h"

#define OK_STR "s05bpd1bps1bpn64A100-s1sf3B10-C34"
#define OK_STR_ENC "s5bpd1bps1bpn64A100-s1sf3B10-C34"

static int get_tag_count(const script *scr, int frame_id) {
    script_frame *frame = vector_get(&scr->frames, frame_id);
    CU_ASSERT_PTR_NOT_NULL(frame);
    return vector_size(&frame->tags);
}

static void script_open_ok(script *s) {
    script_create(s);
    CU_ASSERT_FATAL(script_decode(s, OK_STR, NULL) == SD_SUCCESS);
}

void test_script_create(void) {
    script s;
    script_create(&s);
    CU_ASSERT(vector_size(&s.frames) == 0);
    script_free(&s);
}

void test_script_decode(void) {
    script s;
    script_create(&s);
    CU_ASSERT(script_decode(&s, OK_STR, NULL) == SD_SUCCESS);
    CU_ASSERT(vector_size(&s.frames) == 3);
    CU_ASSERT(get_tag_count(&s, 0) == 4);
    CU_ASSERT(get_tag_count(&s, 1) == 2);
    CU_ASSERT(get_tag_count(&s, 2) == 0);
    script_free(&s);
}

void test_total_ticks(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_get_total_ticks(&s) == 144);
    script_free(&s);

    // New parser should return 0
    script m_script;
    script_create(&m_script);
    CU_ASSERT(script_get_total_ticks(&m_script) == 0);
    script_free(&m_script);
}

void test_tick_pos_at_frame(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_get_tick_pos_at_frame(&s, 0) == 0);
    CU_ASSERT(script_get_tick_pos_at_frame(&s, 1) == 100);
    CU_ASSERT(script_get_tick_pos_at_frame(&s, 2) == 110);
    script_free(&s);
}

void test_tick_len_at_frame(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_get_tick_len_at_frame(&s, 0) == 100);
    CU_ASSERT(script_get_tick_len_at_frame(&s, 1) == 10);
    CU_ASSERT(script_get_tick_len_at_frame(&s, 2) == 34);
    script_free(&s);
}

void test_script_encode(void) {
    script s;
    script_open_ok(&s);
    str dst;
    str_create(&dst);
    CU_ASSERT(script_encode(&s, &dst) == SD_SUCCESS);
    CU_ASSERT(strcmp(OK_STR_ENC, str_c(&dst)) == 0);
    str_free(&dst);
    script_free(&s);
}

void test_script_encode_frame(void) {
    str dst;
    script_frame frame;

    str_create(&dst);
    script_frame_create(&frame, 100, 0);
    script_frame_add_tag(&frame, "bpn", 1);
    script_frame_add_tag(&frame, "m", 12);

    CU_ASSERT(script_encode_frame(&frame, &dst) == SD_SUCCESS);
    CU_ASSERT(strcmp("bpn1m12A100", str_c(&dst)) == 0);

    script_frame_free(&frame);
    str_free(&dst);
}

void test_script_get_frame(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_get_frame(&s, -1) == NULL);
    CU_ASSERT(script_get_frame(&s, 0) != NULL);
    CU_ASSERT(script_get_frame(&s, 1) != NULL);
    CU_ASSERT(script_get_frame(&s, 2) != NULL);
    CU_ASSERT(script_get_frame(&s, 3) == NULL);
    script_free(&s);
}

void test_script_get_frame_at(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_get_frame_at(&s, -1) == NULL);
    CU_ASSERT(script_get_frame_at(&s, 0) == script_get_frame(&s, 0));
    CU_ASSERT(script_get_frame_at(&s, 99) == script_get_frame(&s, 0));
    CU_ASSERT(script_get_frame_at(&s, 100) == script_get_frame(&s, 1));
    CU_ASSERT(script_get_frame_at(&s, 109) == script_get_frame(&s, 1));
    CU_ASSERT(script_get_frame_at(&s, 110) == script_get_frame(&s, 2));
    CU_ASSERT(script_get_frame_at(&s, 143) == script_get_frame(&s, 2));
    CU_ASSERT(script_get_frame_at(&s, 144) == NULL);
    script_free(&s);
}

void test_script_get_tag_by_name(void) {
    script s;
    script_open_ok(&s);
    const script_frame *frame = script_get_frame(&s, 0);
    CU_ASSERT(script_get_tag_by_name(frame, NULL) == NULL);
    CU_ASSERT(script_get_tag_by_name(NULL, "a") == NULL);
    CU_ASSERT(script_get_tag_by_name(frame, "bpd") != NULL);
    CU_ASSERT(script_get_tag_by_name(frame, "bps") != NULL);
    CU_ASSERT(script_get_tag_by_name(frame, "bpn") != NULL);
    CU_ASSERT(script_get_tag_by_name(frame, "mp") == NULL);
    CU_ASSERT(script_get_tag_by_name(frame, "forkingandcountry") == NULL);
    script_free(&s);
}

void test_script_frame_changed(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_frame_changed(&s, -1, 0) == 1);
    CU_ASSERT(script_frame_changed(&s, 0, 0) == 0);
    CU_ASSERT(script_frame_changed(&s, 0, 1) == 0);
    CU_ASSERT(script_frame_changed(&s, 143, 144) == 1);
    CU_ASSERT(script_frame_changed(&s, 99, 100) == 1);
    CU_ASSERT(script_frame_changed(&s, 98, 99) == 0);
    script_free(&s);
}

void test_script_get_frame_index(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_get_frame_index(NULL, NULL) == -1);
    CU_ASSERT(script_get_frame_index(&s, NULL) == -1);
    CU_ASSERT(script_get_frame_index(&s, script_get_frame(&s, -1)) == -1);
    CU_ASSERT(script_get_frame_index(&s, script_get_frame(&s, 0)) == 0);
    CU_ASSERT(script_get_frame_index(&s, script_get_frame(&s, 1)) == 1);
    CU_ASSERT(script_get_frame_index(&s, script_get_frame(&s, 2)) == 2);
    CU_ASSERT(script_get_frame_index(&s, script_get_frame(&s, 3)) == -1);
    script_free(&s);
}

void test_script_get_frame_index_at(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_get_frame_index_at(NULL, ~0u) == -1);
    CU_ASSERT(script_get_frame_index_at(&s, ~0u) == -1);
    CU_ASSERT(script_get_frame_index_at(&s, 0) == 0);
    CU_ASSERT(script_get_frame_index_at(&s, 99) == 0);
    CU_ASSERT(script_get_frame_index_at(&s, 100) == 1);
    CU_ASSERT(script_get_frame_index_at(&s, 144) == -1);
    script_free(&s);
}

void test_is_last_frame(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_is_last_frame(NULL, NULL) == 0);
    CU_ASSERT(script_is_last_frame(&s, script_get_frame(&s, -1)) == 0);
    CU_ASSERT(script_is_last_frame(&s, script_get_frame(&s, 0)) == 0);
    CU_ASSERT(script_is_last_frame(&s, script_get_frame(&s, 2)) == 1);
    CU_ASSERT(script_is_last_frame(&s, script_get_frame(&s, 3)) == 0);
    script_free(&s);
}

void test_is_last_frame_at(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_is_last_frame_at(NULL, 0) == 0);
    CU_ASSERT(script_is_last_frame_at(&s, -1) == 0);
    CU_ASSERT(script_is_last_frame_at(&s, 0) == 0);
    CU_ASSERT(script_is_last_frame_at(&s, 100) == 0);
    CU_ASSERT(script_is_last_frame_at(&s, 110) == 1);
    CU_ASSERT(script_is_last_frame_at(&s, 143) == 1);
    CU_ASSERT(script_is_last_frame_at(&s, 144) == 0);
    script_free(&s);
}

void test_is_first_frame(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_is_first_frame(NULL, NULL) == 0);
    CU_ASSERT(script_is_first_frame(&s, script_get_frame(&s, -1)) == 0);
    CU_ASSERT(script_is_first_frame(&s, script_get_frame(&s, 0)) == 1);
    CU_ASSERT(script_is_first_frame(&s, script_get_frame(&s, 2)) == 0);
    CU_ASSERT(script_is_first_frame(&s, script_get_frame(&s, 3)) == 0);
    script_free(&s);
}

void test_is_first_frame_at(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_is_first_frame_at(NULL, 0) == 0);
    CU_ASSERT(script_is_first_frame_at(&s, -1) == 0);
    CU_ASSERT(script_is_first_frame_at(&s, 0) == 1);
    CU_ASSERT(script_is_first_frame_at(&s, 99) == 1);
    CU_ASSERT(script_is_first_frame_at(&s, 100) == 0);
    CU_ASSERT(script_is_first_frame_at(&s, 110) == 0);
    CU_ASSERT(script_is_first_frame_at(&s, 143) == 0);
    CU_ASSERT(script_is_first_frame_at(&s, 144) == 0);
    script_free(&s);
}

void test_script_is_tag_set_by_name(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_is_tag_set_by_name(NULL, "bps") == 0);
    CU_ASSERT(script_is_tag_set_by_name(script_get_frame(&s, 0), "bps") == 1);
    CU_ASSERT(script_is_tag_set_by_name(script_get_frame(&s, 0), "bpd") == 1);
    CU_ASSERT(script_is_tag_set_by_name(script_get_frame(&s, 0), "mp") == 0);
    script_free(&s);
}

void test_script_get_tag_value_by_name(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_get_tag_value_by_name(NULL, "bps") == 0);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "bps") == 1);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "bpd") == 1);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "bpn") == 64);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "mp") == 0);
    script_free(&s);
}

void test_script_tag_vars(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "s") == 5); // 05 -> 5 should work
    script_free(&s);
}

void test_tag_lookup(void) {
    script_tag tag;

    // Single, double and triple letter tags map to the correct enum
    CU_ASSERT(script_tag_lookup("m", 1, &tag) && tag == TAG_M);
    CU_ASSERT(script_tag_lookup("bm", 2, &tag) && tag == TAG_BM);
    CU_ASSERT(script_tag_lookup("bpd", 3, &tag) && tag == TAG_BPD);

    // Special non-letter tags
    CU_ASSERT(script_tag_lookup("x-", 2, &tag) && tag == TAG_X_MINUS);
    CU_ASSERT(script_tag_lookup("y=", 2, &tag) && tag == TAG_Y_EQ);

    // Length must be respected
    CU_ASSERT(script_tag_lookup("bm9", 2, &tag) && tag == TAG_BM);
    CU_ASSERT(script_tag_lookup("bpd", 1, &tag) && tag == TAG_B);

    // Unknown and invalid tags are rejected.
    CU_ASSERT(!script_tag_lookup("qq", 2, NULL));
    CU_ASSERT(!script_tag_lookup("c", 1, NULL));
}

void test_script_all(void) {
    str dst;
    int fail_at;
    for(int i = 0; i < TEST_STRING_COUNT; i++) {
        str_create(&dst);
        script s;
        script_create(&s);
        const int ret = script_decode(&s, test_strings[i], &fail_at);
        if(ret == SD_SUCCESS) {
            CU_ASSERT(script_encode(&s, &dst) == SD_SUCCESS);
        } else {
            printf("%s - (%d - %c)\n", test_strings[i], fail_at, test_strings[i][fail_at]);
            CU_FAIL("Parser failed. Broken string ?");
        }
        script_free(&s);
        str_free(&dst);
    }
}

void test_next_frame_with_sprite(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_get_next_frame_with_sprite(NULL, 0, 0) == -1);  // parser NULL
    CU_ASSERT(script_get_next_frame_with_sprite(&s, -1, 0) == -1);   // nonexistent frame id
    CU_ASSERT(script_get_next_frame_with_sprite(&s, 0, 1000) == -1); // Tick does not exist
    CU_ASSERT(script_get_next_frame_with_sprite(&s, 2, 0) == 2);     // C
    CU_ASSERT(script_get_next_frame_with_sprite(&s, 1, 0) == 1);     // B
    CU_ASSERT(script_get_next_frame_with_sprite(&s, 0, 100) == -1);  // A
    CU_ASSERT(script_get_next_frame_with_sprite(&s, 3, 0) == -1);    // D (does not exist)
    CU_ASSERT(script_get_next_frame_with_sprite(&s, 0, 0) == -1);    // A, exists but should not be found
    CU_ASSERT(script_get_next_frame_with_sprite(&s, 0, 99) == -1);   // A, exists but should not be found
    CU_ASSERT(script_get_next_frame_with_sprite(&s, 1, 99) == 1);    // B
    script_free(&s);
}

void test_next_frame_with_tag(void) {
    script s;
    script_open_ok(&s);
    CU_ASSERT(script_get_next_frame_with_tag(NULL, "s", 0) == -1);  // parser NULL
    CU_ASSERT(script_get_next_frame_with_tag(&s, "xxx", 0) == -1);  // nonexistent tag
    CU_ASSERT(script_get_next_frame_with_tag(&s, "s", 1000) == -1); // tick does not exist
    CU_ASSERT(script_get_next_frame_with_tag(&s, "s", 0) == 1);     // Should be in frame 1
    CU_ASSERT(script_get_next_frame_with_tag(&s, "bpd", 0) == -1);  // should be in frame 0, but should not be found
    CU_ASSERT(script_get_next_frame_with_tag(&s, "sf", 0) == 1);    // Just test any tag
    CU_ASSERT(script_get_next_frame_with_tag(&s, "sf", 99) == 1);   // Border case 1
    CU_ASSERT(script_get_next_frame_with_tag(&s, "sf", 100) == -1); // Border case 2
    script_free(&s);
}

void test_set_tag(void) {
    script s;
    script_open_ok(&s);

    // Test value setting to existing tag
    CU_ASSERT(script_set_tag(&s, 0, "bpd", 10) == SD_SUCCESS);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "bpd") == 10);

    // Test creating new tag and make sure nothing got overwritten
    CU_ASSERT(script_set_tag(&s, 1, "bpd", 50) == SD_SUCCESS);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 1), "bpd") == 50);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 1), "s") == 1);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 1), "sf") == 3);

    // Check tag count
    CU_ASSERT_EQUAL(get_tag_count(&s, 1), 3);

    // Bad input values
    CU_ASSERT(script_set_tag(&s, -1, "bpd", 50) == SD_INVALID_INPUT);
    CU_ASSERT(script_set_tag(&s, 1, "xxx", 50) == SD_INVALID_INPUT);

    script_free(&s);
}

void test_append_frame(void) {
    script s;
    script_create(&s);

    CU_ASSERT(script_append_frame(&s, 100, 0) == SD_SUCCESS);
    CU_ASSERT(script_append_frame(&s, 1, 5) == SD_SUCCESS);

    CU_ASSERT(script_get_frame(&s, 0) != NULL);
    CU_ASSERT(script_get_frame(&s, 1) != NULL);
    CU_ASSERT(script_get_frame(&s, 2) == NULL);

    CU_ASSERT(script_get_tick_len_at_frame(&s, 0) == 100);
    CU_ASSERT(script_get_tick_len_at_frame(&s, 1) == 1);

    script_free(&s);
}

void test_clear_tags(void) {
    script s;

    // Create a test case
    script_create(&s);
    CU_ASSERT(script_append_frame(&s, 100, 0) == SD_SUCCESS);
    CU_ASSERT(script_set_tag(&s, 0, "bpd", 10) == SD_SUCCESS);

    // Real tests
    CU_ASSERT(script_clear_tags(&s, 0) == SD_SUCCESS);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "bpd") == 0);

    script_free(&s);
}

void test_delete_tag(void) {
    script s;

    // Create a test case
    script_create(&s);
    CU_ASSERT(script_append_frame(&s, 100, 0) == SD_SUCCESS);
    CU_ASSERT(script_set_tag(&s, 0, "bpn", 10) == SD_SUCCESS);
    CU_ASSERT(get_tag_count(&s, 0) == 1);
    CU_ASSERT(script_set_tag(&s, 0, "s", 15) == SD_SUCCESS);
    CU_ASSERT(get_tag_count(&s, 0) == 2);
    CU_ASSERT(script_set_tag(&s, 0, "sf", 100) == SD_SUCCESS);
    CU_ASSERT(get_tag_count(&s, 0) == 3);

    // Real tests
    CU_ASSERT(script_delete_tag(&s, 0, "bpn") == SD_SUCCESS);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "bpn") == 0);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "s") == 15);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "sf") == 100);
    CU_ASSERT(get_tag_count(&s, 0) == 2);

    CU_ASSERT(script_delete_tag(&s, 0, "s") == SD_SUCCESS);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "bpn") == 0);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "s") == 0);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "sf") == 100);
    CU_ASSERT(get_tag_count(&s, 0) == 1);

    CU_ASSERT(script_delete_tag(&s, 0, "sf") == SD_SUCCESS);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "bpn") == 0);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "s") == 0);
    CU_ASSERT(script_get_tag_value_by_name(script_get_frame(&s, 0), "sf") == 0);
    CU_ASSERT(get_tag_count(&s, 0) == 0);

    script_free(&s);
}

void test_set_tick_len_at_frame(void) {
    script s;

    // Create a test case
    script_create(&s);
    CU_ASSERT(script_append_frame(&s, 100, 0) == SD_SUCCESS);

    // Real tests
    CU_ASSERT(script_set_tick_len_at_frame(&s, 0, 500) == SD_SUCCESS);
    CU_ASSERT(script_get_tick_len_at_frame(&s, 0) == 500);

    script_free(&s);
}

void test_set_sprite_at_frame(void) {
    script s;

    // Create a test case
    script_create(&s);
    CU_ASSERT(script_append_frame(&s, 100, 0) == SD_SUCCESS);

    // Real tests
    CU_ASSERT(script_set_sprite_at_frame(&s, 0, script_letter_to_frame('Z')) == SD_SUCCESS);
    CU_ASSERT(script_get_sprite_at_frame(&s, 0) == 25);
    CU_ASSERT(script_set_sprite_at_frame(&s, 0, script_letter_to_frame('A')) == SD_SUCCESS);
    CU_ASSERT(script_get_sprite_at_frame(&s, 0) == 0);
    CU_ASSERT(script_set_sprite_at_frame(&s, 0, 26) == SD_INVALID_INPUT);
    CU_ASSERT(script_get_sprite_at_frame(&s, 0) == 0);

    script_free(&s);
}

void test_letter_to_frame(void) {
    CU_ASSERT(script_letter_to_frame('A') == 0);
    CU_ASSERT(script_letter_to_frame('Z') == 25);
}

void test_frame_to_letter(void) {
    CU_ASSERT(script_frame_to_letter(0) == 'A');
    CU_ASSERT(script_frame_to_letter(25) == 'Z');
}

void script_test_suite(CU_pSuite suite) {
    ADD_TEST("test of script_create", test_script_create);
    ADD_TEST("test of script_decode", test_script_decode);
    ADD_TEST("test of script_get_total_ticks", test_total_ticks);
    ADD_TEST("test of script_get_tick_pos_at_frame", test_tick_pos_at_frame);
    ADD_TEST("test of script_get_tick_len_at_frame", test_tick_len_at_frame);
    ADD_TEST("test of script_encode", test_script_encode);
    ADD_TEST("test of script_get_frame", test_script_get_frame);
    ADD_TEST("test of script_get_frame_at", test_script_get_frame_at);
    ADD_TEST("test of script_get_tag_by_name", test_script_get_tag_by_name);
    ADD_TEST("test of script_frame_changed", test_script_frame_changed);
    ADD_TEST("test of script_get_frame_index", test_script_get_frame_index);
    ADD_TEST("test of script_get_frame_index_at", test_script_get_frame_index_at);
    ADD_TEST("test of script_is_last_frame", test_is_last_frame);
    ADD_TEST("test of script_is_last_frame_at", test_is_last_frame_at);
    ADD_TEST("test of script_is_first_frame", test_is_first_frame);
    ADD_TEST("test of script_is_first_frame_at", test_is_first_frame_at);
    ADD_TEST("test of script_is_tag_set_by_name", test_script_is_tag_set_by_name);
    ADD_TEST("test of script_get_tag_value_by_name", test_script_get_tag_value_by_name);
    ADD_TEST("test of script_get_next_frame_with_sprite", test_next_frame_with_sprite);
    ADD_TEST("test of script_get_next_frame_with_tag", test_next_frame_with_tag);
    ADD_TEST("test of script_set_tag", test_set_tag);
    ADD_TEST("test of script_delete_tag", test_delete_tag);
    ADD_TEST("test of script_append_frame", test_append_frame);
    ADD_TEST("test of script_clear_tags", test_clear_tags);
    ADD_TEST("test of script_set_tick_len_at_frame", test_set_tick_len_at_frame);
    ADD_TEST("test of script_set_sprite_at_frame", test_set_sprite_at_frame);
    ADD_TEST("test of script_letter_to_frame", test_letter_to_frame);
    ADD_TEST("test of script_frame_to_letter", test_frame_to_letter);
    ADD_TEST("test of script_encode_frame", test_script_encode_frame);
    ADD_TEST("testing odd tags", test_script_tag_vars);
    ADD_TEST("test of tag_lookup", test_tag_lookup);
    ADD_TEST("test of all OMF strings", test_script_all);
}
