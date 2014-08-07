#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <shadowdive/shadowdive.h>

sd_af_file af;

void test_sd_af_create(void) {
    int ret;
    ret = sd_af_create(&af);
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_af_create(NULL);
    CU_ASSERT(ret == SD_INVALID_INPUT);
}

void test_sd_af_free(void) {
    sd_af_free(&af);
}

void test_af_roundtrip(void) {
    sd_af_file new;
    sd_af_file loaded;
    sd_move move;
    sd_animation ani;
    int ret;

    ret = sd_af_create(&new);
    CU_ASSERT(ret == SD_SUCCESS);
    
    // Set some values
    new.file_id = 1;
    new.unknown_a = 2;
    new.endurance = 3;
    new.unknown_b = 4;
    new.power = 5;
    new.forward_speed = 6;
    new.reverse_speed = 7;
    new.jump_speed = 8;
    new.fall_speed = 9;
    new.unknown_c = 10;
    new.unknown_d = 11;
    memset(new.soundtable, 10, sizeof(new.soundtable));

    // Create a new move
    ret = sd_move_create(&move);
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_move_set_footer_string(&move, "A10-B10-C10");
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_move_set_move_string(&move, "D10-E10");
    CU_ASSERT(ret == SD_SUCCESS);

    // Create a new animation for move
    ret = sd_animation_create(&ani);
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_animation_set_anim_string(&ani, "F10-G10-H10");
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_animation_push_extra_string(&ani, "s10A100");
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_animation_push_extra_string(&ani, "s10B100");
    CU_ASSERT(ret == SD_SUCCESS);

    // Copy animation to move
    ret = sd_move_set_animation(&move, &ani);
    CU_ASSERT(ret == SD_SUCCESS);

    // Copy move to AF file
    //ret = sd_af_set_move(&new, 0, &move);
    //CU_ASSERT(ret == SD_SUCCESS);

    // Roundtripping
    ret = sd_af_create(&loaded);
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_af_save(&new, "test.af");
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_af_load(&loaded, "test.af");
    CU_ASSERT(ret == SD_SUCCESS);

    // Check some random samples from sd_af_file
    CU_ASSERT_EQUAL(loaded.file_id, new.file_id);
    CU_ASSERT_EQUAL(loaded.unknown_c, new.unknown_c);
    CU_ASSERT_EQUAL(loaded.unknown_d, new.unknown_d);
    CU_ASSERT_NSTRING_EQUAL(loaded.soundtable, new.soundtable, 30);

    // Make sure ID 0 exists in moves
   /* CU_ASSERT_PTR_NOT_NULL(new.moves[0]);
    CU_ASSERT_PTR_NOT_NULL(loaded.moves[0]);

    // Check strings from move 0
    CU_ASSERT_STRING_EQUAL(new.moves[0]->move_string, loaded.moves[0]->move_string);
    CU_ASSERT_STRING_EQUAL(new.moves[0]->footer_string, loaded.moves[0]->footer_string);

    // Check that animation seems correct
    CU_ASSERT_STRING_EQUAL(new.moves[0]->animation->anim_string, loaded.moves[0]->animation->anim_string);
    CU_ASSERT(new.moves[0]->animation->extra_string_count == 2);*/

    sd_af_free(&new);
    sd_af_free(&loaded);
}

void af_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "test of sd_af_create", test_sd_af_create) == NULL) { return; }
    if(CU_add_test(suite, "test of AF empty roundtripping", test_af_roundtrip) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_af_free", test_sd_af_free) == NULL) { return; }
}