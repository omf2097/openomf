#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <shadowdive/shadowdive.h>

sd_tournament_file trn;

void test_sd_trn_create(void) {
    CU_ASSERT(sd_tournament_create(&trn) == SD_SUCCESS);
    CU_ASSERT(sd_tournament_create(NULL) == SD_INVALID_INPUT);
}

void test_sd_trn_free(void) {
    sd_tournament_free(&trn);
}

void trn_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "test of sd_trn_create", test_sd_trn_create) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_trn_free", test_sd_trn_free) == NULL) { return; }
}