#include "common.h"
#include "formats/bk.h"
#include "formats/error.h"

sd_bk_file bk;

void test_sd_bk_create(void) {
    sd_bk_create(&bk);
}

void test_sd_bk_free(void) {
    sd_bk_free(&bk);
}

void test_bk_roundtrip(void) {
    sd_bk_file new;
    sd_bk_file loaded;
    int ret;

    sd_bk_create(&new);

    // Set some values
    new.file_id = 1;
    new.unknown_a = 2;
    memset(new.soundtable, 10, sizeof(new.soundtable));

    path test_file;
    path_from_c(&test_file, "test.bk");

    // Roundtripping
    sd_bk_create(&loaded);
    ret = sd_bk_save(&new, &test_file);
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_bk_load(&loaded, &test_file);
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
    ADD_TEST("test of sd_bk_create", test_sd_bk_create);
    ADD_TEST("test of BK empty roundtripping", test_bk_roundtrip);
    ADD_TEST("test of sd_bk_free", test_sd_bk_free);
}
