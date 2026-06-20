#include "common.h"
#include "formats/error.h"
#include "formats/rec.h"
#include <stdio.h>
#include <stdlib.h>

sd_rec_file rec;

void fill(char *buf, int len) {
    uint8_t key = 0;
    for(int i = 0; i < len; i++) {
        buf[i] = key++;
    }
}

void test_sd_rec_create(void) {
    sd_rec_create(&rec);

    // Set some values
    for(int i = 0; i < 10; i++) {
        sd_rec_move mv;
        fill((char *)&mv, sizeof(sd_rec_move));
        smallbuffer_create(&mv.extra_data);
        char *extra_data = sd_rec_set_lookup_id(&mv, 2);
        extra_data[0] = SD_ACT_KICK | SD_ACT_PUNCH | SD_ACT_DOWNLEFT;
        sd_rec_insert_action(&rec, i, &mv);
    }
}

void test_sd_rec_free(void) {
    sd_rec_free(&rec);
}

void test_rec_roundtrip(void) {
    sd_rec_file loaded;

    path test_file;
    path_from_c(&test_file, "test.rec");

    // Roundtripping
    sd_rec_create(&loaded);
    CU_ASSERT(sd_rec_save(&rec, &test_file) == SD_SUCCESS);
    CU_ASSERT(sd_rec_load(&loaded, &test_file) == SD_SUCCESS);

    // Make sure the RECs seem the same
    CU_ASSERT(vector_size(&rec.moves) == vector_size(&loaded.moves));
    for(unsigned i = 0; i < vector_size(&rec.moves); i++) {
        sd_rec_move *rec_move = vector_get(&rec.moves, i);
        sd_rec_move *loaded_move = vector_get(&loaded.moves, i);
        CU_ASSERT(rec_move->tick == loaded_move->tick);
        CU_ASSERT(rec_move->lookup_id == loaded_move->lookup_id);
        CU_ASSERT(rec_move->player_id == loaded_move->player_id);

        int len = sd_rec_extra_len(rec_move->lookup_id);
        if(len) {
            CU_ASSERT(memcmp(sd_rec_get_extra_data(rec_move), sd_rec_get_extra_data(loaded_move), len) == 0);
        }
    }

    // Test pilot data
    CU_ASSERT_NSTRING_EQUAL((char *)&rec.pilots[0], (char *)&loaded.pilots[0], sizeof(loaded.pilots[0]));
    CU_ASSERT_NSTRING_EQUAL((char *)&rec.pilots[1], (char *)&loaded.pilots[1], sizeof(loaded.pilots[1]));

    // Free the loaded struct
    sd_rec_free(&loaded);
}

void test_crystal_shirro_load(void) {
    path test_path;
    path_from_parts(&test_path, TESTS_ROOT_DIR, "recs", "crystal-shirro.rec");
    sd_rec_create(&rec);
    CU_ASSERT(sd_rec_load(&rec, &test_path) == SD_SUCCESS);
    sd_rec_free(&rec);
}

void rec_test_suite(CU_pSuite suite) {
    ADD_TEST("test of sd_rec_create", test_sd_rec_create);
    ADD_TEST("test of REC roundtripping", test_rec_roundtrip);
    ADD_TEST("test of sd_rec_free", test_sd_rec_free);
    ADD_TEST("test loading crystal-shirro.rec", test_crystal_shirro_load);
}
