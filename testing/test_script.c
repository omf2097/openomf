#include "formats/error.h"
#include "formats/script.h"
#include "misc/parser_test_strings.h"
#include <CUnit/CUnit.h>

sd_script script;

#define OK_STR "s05bpd1bps1bpn64A100-s1sf3B10-C34"
#define OK_STR_ENC "s5bpd1bps1bpn64A100-s1sf3B10-C34"

static int get_tag_count(const sd_script *scr, int frame_id) {
    sd_script_frame *frame = vector_get(&scr->frames, frame_id);
    CU_ASSERT_PTR_NOT_NULL(frame);
    return vector_size(&frame->tags);
}

void test_script_create(void) {
    CU_ASSERT(sd_script_create(&script) == SD_SUCCESS);
    CU_ASSERT(sd_script_create(NULL) == SD_INVALID_INPUT);
    CU_ASSERT(vector_size(&script.frames) == 0);
}

void test_script_decode(void) {
    CU_ASSERT(sd_script_decode(&script, NULL, NULL) == SD_INVALID_INPUT);
    CU_ASSERT(sd_script_decode(NULL, OK_STR, NULL) == SD_INVALID_INPUT);
    CU_ASSERT(sd_script_decode(&script, OK_STR, NULL) == SD_SUCCESS);
    CU_ASSERT(vector_size(&script.frames) == 3);
    CU_ASSERT(get_tag_count(&script, 0) == 4);
    CU_ASSERT(get_tag_count(&script, 1) == 2);
    CU_ASSERT(get_tag_count(&script, 2) == 0);
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

void test_script_encode(void) {
    str dst;
    str_create(&dst);
    CU_ASSERT(sd_script_encode(&script, &dst) == SD_SUCCESS);
    CU_ASSERT(strcmp(OK_STR_ENC, str_c(&dst)) == 0);
    str_free(&dst);
}

void test_script_encode_frame(void) {
    str dst;
    sd_script_frame frame;

    str_create(&dst);
    sd_script_frame_create(&frame, 100, 0);
    sd_script_frame_add_tag(&frame, "bpn", 1);
    sd_script_frame_add_tag(&frame, "m", 12);

    CU_ASSERT(sd_script_encode_frame(&frame, &dst) == SD_SUCCESS);
    CU_ASSERT(strcmp("bpn1m12A100", str_c(&dst)) == 0);

    sd_script_frame_free(&frame);
    str_free(&dst);
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
    CU_ASSERT(sd_script_frame_changed(&script, 0, 0) == 0);
    CU_ASSERT(sd_script_frame_changed(&script, 0, 1) == 0);
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
    CU_ASSERT(sd_script_get_frame_index_at(NULL, ~0u) == -1);
    CU_ASSERT(sd_script_get_frame_index_at(&script, ~0u) == -1);
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
    str dst;
    int fail_at;
    for(int i = 0; i < TEST_STRING_COUNT; i++) {
        str_create(&dst);
        sd_script s;
        CU_ASSERT_FATAL(sd_script_create(&s) == SD_SUCCESS);
        int ret = sd_script_decode(&s, test_strings[i], &fail_at);
        if(ret == SD_SUCCESS) {
            CU_ASSERT(sd_script_encode(&s, &dst) == SD_SUCCESS);
        } else {
            printf("%s - (%d - %c)\n", test_strings[i], fail_at, test_strings[i][fail_at]);
            CU_FAIL("Parser failed. Broken string ?");
        }
        sd_script_free(&s);
        str_free(&dst);
    }
}

void test_next_frame_with_sprite(void) {
    CU_ASSERT(sd_script_next_frame_with_sprite(NULL, 0, 0) == -1);       // script NULL
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, -1, 0) == -1);   // nonexistent frame id
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 0, 1000) == -1); // Tick does not exist
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 2, 0) == 2);     // C
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 1, 0) == 1);     // B
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 0, 100) == -1);  // A
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 3, 0) == -1);    // D (does not exist)
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 0, 0) == -1);    // A, exists but should not be found
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 0, 99) == -1);   // A, exists but should not be found
    CU_ASSERT(sd_script_next_frame_with_sprite(&script, 1, 99) == 1);    // B
}

void test_next_frame_with_tag(void) {
    CU_ASSERT(sd_script_next_frame_with_tag(NULL, "s", 0) == -1);       // script NULL
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "xxx", 0) == -1);  // nonexistent tag
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "s", 1000) == -1); // tick does not exist
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "s", 0) == 1);     // Should be in frame 1
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "bpd", 0) == -1);  // should be in frame 0, but should not be found
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "sf", 0) == 1);    // Just test any tag
    CU_ASSERT(sd_script_next_frame_with_tag(&script, "sf", 99) == 1);   // Border case 1
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
    CU_ASSERT_EQUAL(get_tag_count(&script, 1), 3);

    // Bad input values
    CU_ASSERT(sd_script_set_tag(NULL, 1, "bpd", 50) == SD_INVALID_INPUT);
    CU_ASSERT(sd_script_set_tag(&script, -1, "bpd", 50) == SD_INVALID_INPUT);
    CU_ASSERT(sd_script_set_tag(&script, 1, "xxx", 50) == SD_INVALID_INPUT);
}

void test_append_frame(void) {
    sd_script s;
    CU_ASSERT(sd_script_create(&s) == SD_SUCCESS);

    CU_ASSERT(sd_script_append_frame(&s, 100, 0) == SD_SUCCESS);
    CU_ASSERT(sd_script_append_frame(&s, 1, 5) == SD_SUCCESS);

    CU_ASSERT(sd_script_get_frame(&s, 0) != NULL);
    CU_ASSERT(sd_script_get_frame(&s, 1) != NULL);
    CU_ASSERT(sd_script_get_frame(&s, 2) == NULL);

    CU_ASSERT(sd_script_get_tick_len_at_frame(&s, 0) == 100);
    CU_ASSERT(sd_script_get_tick_len_at_frame(&s, 1) == 1);

    sd_script_free(&s);
}

void test_clear_tags(void) {
    sd_script s;

    // Create a test case
    CU_ASSERT(sd_script_create(&s) == SD_SUCCESS);
    CU_ASSERT(sd_script_append_frame(&s, 100, 0) == SD_SUCCESS);
    CU_ASSERT(sd_script_set_tag(&s, 0, "bpd", 10) == SD_SUCCESS);

    // Real tests
    CU_ASSERT(sd_script_clear_tags(&s, 0) == SD_SUCCESS);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&s, 0), "bpd") == 0);

    sd_script_free(&s);
}

void test_delete_tag(void) {
    sd_script s;

    // Create a test case
    CU_ASSERT(sd_script_create(&s) == SD_SUCCESS);
    CU_ASSERT(sd_script_append_frame(&s, 100, 0) == SD_SUCCESS);
    CU_ASSERT(sd_script_set_tag(&s, 0, "bpn", 10) == SD_SUCCESS);
    CU_ASSERT(get_tag_count(&s, 0) == 1);
    CU_ASSERT(sd_script_set_tag(&s, 0, "s", 15) == SD_SUCCESS);
    CU_ASSERT(get_tag_count(&s, 0) == 2);
    CU_ASSERT(sd_script_set_tag(&s, 0, "sf", 100) == SD_SUCCESS);
    CU_ASSERT(get_tag_count(&s, 0) == 3);

    // Real tests
    CU_ASSERT(sd_script_delete_tag(&s, 0, "bpn") == SD_SUCCESS);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&s, 0), "bpn") == 0);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&s, 0), "s") == 15);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&s, 0), "sf") == 100);
    CU_ASSERT(get_tag_count(&s, 0) == 2);

    CU_ASSERT(sd_script_delete_tag(&s, 0, "s") == SD_SUCCESS);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&s, 0), "bpn") == 0);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&s, 0), "s") == 0);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&s, 0), "sf") == 100);
    CU_ASSERT(get_tag_count(&s, 0) == 1);

    CU_ASSERT(sd_script_delete_tag(&s, 0, "sf") == SD_SUCCESS);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&s, 0), "bpn") == 0);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&s, 0), "s") == 0);
    CU_ASSERT(sd_script_get(sd_script_get_frame(&s, 0), "sf") == 0);
    CU_ASSERT(get_tag_count(&s, 0) == 0);

    sd_script_free(&s);
}

void test_set_tick_len_at_frame(void) {
    sd_script s;

    // Create a test case
    CU_ASSERT(sd_script_create(&s) == SD_SUCCESS);
    CU_ASSERT(sd_script_append_frame(&s, 100, 0) == SD_SUCCESS);

    // Real tests
    CU_ASSERT(sd_script_set_tick_len_at_frame(&s, 0, 500) == SD_SUCCESS);
    CU_ASSERT(sd_script_get_tick_len_at_frame(&s, 0) == 500);

    sd_script_free(&s);
}

void test_set_sprite_at_frame(void) {
    sd_script s;

    // Create a test case
    CU_ASSERT(sd_script_create(&s) == SD_SUCCESS);
    CU_ASSERT(sd_script_append_frame(&s, 100, 0) == SD_SUCCESS);

    // Real tests
    CU_ASSERT(sd_script_set_sprite_at_frame(&s, 0, sd_script_letter_to_frame('Z')) == SD_SUCCESS);
    CU_ASSERT(sd_script_get_sprite_at_frame(&s, 0) == 25);
    CU_ASSERT(sd_script_set_sprite_at_frame(&s, 0, sd_script_letter_to_frame('A')) == SD_SUCCESS);
    CU_ASSERT(sd_script_get_sprite_at_frame(&s, 0) == 0);
    CU_ASSERT(sd_script_set_sprite_at_frame(&s, 0, 26) == SD_INVALID_INPUT);
    CU_ASSERT(sd_script_get_sprite_at_frame(&s, 0) == 0);

    sd_script_free(&s);
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
    if(CU_add_test(suite, "test of sd_script_create", test_script_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_decode", test_script_decode) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_get_total_ticks", test_total_ticks) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_get_tick_pos_at_frame", test_tick_pos_at_frame) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_get_tick_len_at_frame", test_tick_len_at_frame) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_encode", test_script_encode) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_get_frame", test_script_get_frame) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_get_frame_at", test_script_get_frame_at) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_get_tag", test_script_get_tag) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_frame_changed", test_script_frame_changed) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_get_frame_index", test_script_get_frame_index) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_get_frame_index_at", test_script_get_frame_index_at) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_is_last_frame", test_is_last_frame) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_is_last_frame_at", test_is_last_frame_at) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_is_first_frame", test_is_first_frame) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_is_first_frame_at", test_is_first_frame_at) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_isset", test_script_isset) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_get", test_script_get) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_next_frame_with_sprite", test_next_frame_with_sprite) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_next_frame_with_tag", test_next_frame_with_tag) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_set_tag", test_set_tag) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_delete_tag", test_delete_tag) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_append_frame", test_append_frame) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_clear_tags", test_clear_tags) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_set_tick_len_at_frame", test_set_tick_len_at_frame) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_set_sprite_at_frame", test_set_sprite_at_frame) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_letter_to_frame", test_letter_to_frame) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_frame_to_letter", test_frame_to_letter) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_encode_frame", test_script_encode_frame) == NULL) {
        return;
    }
    if(CU_add_test(suite, "testing odd tags", test_script_tag_vars) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_script_free", test_script_free) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of all OMF strings", test_script_all) == NULL) {
        return;
    }
}
