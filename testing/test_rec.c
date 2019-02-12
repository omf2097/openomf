#include <stdio.h>
#include <stdlib.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "formats/rec.h"
#include "formats/error.h"

sd_rec_file rec;

void fill(char *buf, int len) {
    uint8_t key = 0;
    for(int i = 0; i < len; i++) {
        buf[i] = key++;
    }
}

void test_sd_rec_create(void) {
    CU_ASSERT(sd_rec_create(&rec) == SD_SUCCESS);
    CU_ASSERT(sd_rec_create(NULL) == SD_INVALID_INPUT);

    // Set some values
    for(int i = 0; i < 10; i++) {
        sd_rec_move mv;
        fill((char*)&mv, sizeof(sd_rec_move));
        mv.lookup_id = 2;
        mv.action = SD_ACT_KICK|SD_ACT_PUNCH|SD_ACT_LEFT|SD_ACT_DOWN;
        mv.extra_data = NULL;
        sd_rec_insert_action(&rec, i, &mv);
    }
}

void test_sd_rec_free(void) {
    sd_rec_free(&rec);
}

void test_rec_roundtrip(void) {
    sd_rec_file loaded;

    // Roundtripping
    CU_ASSERT(sd_rec_create(&loaded) == SD_SUCCESS);
    CU_ASSERT(sd_rec_save(&rec, "test.rec") == SD_SUCCESS);
    CU_ASSERT(sd_rec_load(&loaded, "test.rec") == SD_SUCCESS);

    // Make sure the RECs seem the same
    CU_ASSERT(rec.move_count == loaded.move_count);
    for(int i = 0; i < rec.move_count; i++) {
        CU_ASSERT(rec.moves[i].tick == loaded.moves[i].tick);
        CU_ASSERT(rec.moves[i].lookup_id == loaded.moves[i].lookup_id);
        CU_ASSERT(rec.moves[i].player_id == loaded.moves[i].player_id);

        if(rec.moves[i].lookup_id > 2) {
            CU_ASSERT(rec.moves[i].raw_action == loaded.moves[i].raw_action);
            CU_ASSERT_NSTRING_EQUAL(
                rec.moves[i].extra_data,
                loaded.moves[i].extra_data,
                7);
        } else {
            CU_ASSERT(rec.moves[i].action == loaded.moves[i].action);
        }
    }

    // Test pilot data
    CU_ASSERT_NSTRING_EQUAL(
        (char*)&rec.pilots[0], (char*)&loaded.pilots[0], sizeof(loaded.pilots[0]));
    CU_ASSERT_NSTRING_EQUAL(
        (char*)&rec.pilots[1], (char*)&loaded.pilots[1], sizeof(loaded.pilots[1]));

    // Free the loaded struct
    sd_rec_free(&loaded);
}

void rec_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "test of sd_rec_create", test_sd_rec_create) == NULL) { return; }
    if(CU_add_test(suite, "test of REC roundtripping", test_rec_roundtrip) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_rec_free", test_sd_rec_free) == NULL) { return; }
}