#include "formats/error.h"
#include "formats/tournament.h"
#include "utils/allocator.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdio.h>
#include <stdlib.h>

sd_tournament_file trn;

void test_sd_trn_create(void) {
    CU_ASSERT(sd_tournament_create(&trn) == SD_SUCCESS);
    CU_ASSERT(sd_tournament_create(NULL) == SD_INVALID_INPUT);
}

void test_sd_trn_roundtripping(void) {
    sd_tournament_file n_trn;
    sd_tournament_file l_trn;

    CU_ASSERT(sd_tournament_create(&n_trn) == SD_SUCCESS);
    CU_ASSERT(sd_tournament_create(&l_trn) == SD_SUCCESS);

    // Set values
    n_trn.enemy_count = 0;
    n_trn.winnings_multiplier = 0;
    n_trn.unknown_a = 0;
    n_trn.registration_fee = 1000;
    n_trn.assumed_initial_value = 2000;
    n_trn.tournament_id = 1;
    sd_tournament_set_bk_name(&n_trn, "test.bk");
    sd_tournament_set_pic_name(&n_trn, "pilots.pic");

    memset(&n_trn.pal, 64, sizeof(n_trn.pal));

    n_trn.enemies[0] = omf_calloc(1, sizeof(sd_pilot));
    n_trn.enemy_count = 1;
    snprintf(n_trn.enemies[0]->name, 18, "test_pilot");

    CU_ASSERT(sd_tournament_save(&n_trn, "test.trn") == SD_SUCCESS);
    CU_ASSERT(sd_tournament_load(&l_trn, "test.trn") == SD_SUCCESS);

    CU_ASSERT(n_trn.enemy_count == l_trn.enemy_count);
    CU_ASSERT(n_trn.winnings_multiplier == l_trn.winnings_multiplier);
    CU_ASSERT(n_trn.unknown_a == l_trn.unknown_a);
    CU_ASSERT(n_trn.registration_fee == l_trn.registration_fee);
    CU_ASSERT(n_trn.assumed_initial_value == l_trn.assumed_initial_value);
    CU_ASSERT(n_trn.tournament_id == l_trn.tournament_id);

    CU_ASSERT_STRING_EQUAL(n_trn.bk_name, l_trn.bk_name);
    CU_ASSERT_STRING_EQUAL(n_trn.pic_file, l_trn.pic_file);

    CU_ASSERT_STRING_EQUAL(n_trn.enemies[0]->name, l_trn.enemies[0]->name);

    sd_tournament_free(&n_trn);
    sd_tournament_free(&l_trn);
}

void test_sd_trn_free(void) {
    sd_tournament_free(&trn);
}

void trn_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "test of sd_trn_create", test_sd_trn_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test roundtripping", test_sd_trn_roundtripping) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_trn_free", test_sd_trn_free) == NULL) {
        return;
    }
}
