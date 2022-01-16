#include "formats/bk.h"
#include "formats/error.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

sd_bk_file bk;

void test_sd_bk_create(void) {
    int ret;
    ret = sd_bk_create(&bk);
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_bk_create(NULL);
    CU_ASSERT(ret == SD_INVALID_INPUT);
}

void test_sd_bk_free(void) {
    sd_bk_free(&bk);
}

void test_bk_roundtrip(void) {
    sd_bk_file new;
    sd_bk_file loaded;
    int ret;

    ret = sd_bk_create(&new);
    CU_ASSERT(ret == SD_SUCCESS);

    // Set some values
    new.file_id = 1;
    new.unknown_a = 2;
    memset(new.soundtable, 10, sizeof(new.soundtable));

    // Roundtripping
    ret = sd_bk_create(&loaded);
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_bk_save(&new, "test.bk");
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_bk_load(&loaded, "test.bk");
    CU_ASSERT(ret == SD_SUCCESS);

    // Quick & dirty
    // TODO: Something better
    CU_ASSERT_EQUAL(loaded.file_id, new.file_id);
    CU_ASSERT_EQUAL(loaded.unknown_a, new.unknown_a);
    CU_ASSERT_EQUAL(loaded.palette_count, new.palette_count);
    CU_ASSERT_NSTRING_EQUAL(loaded.soundtable, new.soundtable, 30);

    sd_bk_free(&new);
    sd_bk_free(&loaded);
}

void bk_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "test of sd_bk_create", test_sd_bk_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of BK empty roundtripping", test_bk_roundtrip) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of sd_bk_free", test_sd_bk_free) == NULL) {
        return;
    }
}