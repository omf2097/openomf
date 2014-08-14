#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <shadowdive/shadowdive.h>
#include <stdio.h>

#define TESTFILE "test.gpl"
#define TESTFILE2 "test2.gpl"

sd_palette pal;

void test_sd_palette_create(void) {
    int ret;
    ret = sd_palette_create(&pal);
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_palette_create(NULL);
    CU_ASSERT(ret == SD_INVALID_INPUT);

    // Fill with data
    for(int i = 0; i < 256; i++) {
        pal.data[i][0] = i;
        pal.data[i][1] = 256-i;
        pal.data[i][2] = 128;
    }
}

void test_sd_palette_gimp_save(void) {
    int ret;

    ret = sd_palette_to_gimp_palette(&pal, NULL);
    CU_ASSERT(ret == SD_INVALID_INPUT);
    ret = sd_palette_to_gimp_palette(NULL, TESTFILE);
    CU_ASSERT(ret == SD_INVALID_INPUT);
    ret = sd_palette_to_gimp_palette(&pal, TESTFILE);
    CU_ASSERT(ret == SD_SUCCESS);
}

void test_sd_palette_gimp_load(void) {
    int ret;
    FILE *tmp;

    ret = sd_palette_from_gimp_palette(&pal, NULL);
    CU_ASSERT(ret == SD_INVALID_INPUT);
    ret = sd_palette_from_gimp_palette(NULL, TESTFILE);
    CU_ASSERT(ret == SD_INVALID_INPUT);

    ret = sd_palette_from_gimp_palette(&pal, "nonesuchfile.gpl");
    CU_ASSERT(ret == SD_FILE_OPEN_ERROR);

    // Create an invalid file
    tmp = fopen(TESTFILE2, "wb");
    CU_ASSERT(tmp != NULL);
    fclose(tmp);

    // Test invalid file
    ret = sd_palette_from_gimp_palette(&pal, TESTFILE2);
    CU_ASSERT(ret == SD_FILE_INVALID_TYPE);

    // Test good file
    ret = sd_palette_from_gimp_palette(&pal, TESTFILE);
    CU_ASSERT(ret == SD_SUCCESS);
}

void test_sd_palette_free(void) {
    sd_palette_free(&pal);
}

void test_gimp_roundtrip(void) {
    sd_palette new;
    int ret;

    // Create
    ret = sd_palette_create(&new);
    CU_ASSERT(ret == SD_SUCCESS);

    // Write
    ret = sd_palette_to_gimp_palette(&pal, TESTFILE);
    CU_ASSERT(ret == SD_SUCCESS);

    // Read
    ret = sd_palette_from_gimp_palette(&new, TESTFILE);
    CU_ASSERT(ret == SD_SUCCESS);

    // Match
    CU_ASSERT_NSTRING_EQUAL(pal.data, new.data, 256*3);

    // Free up
    sd_palette_free(&new);
}

void palette_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "test of sd_palette_create", test_sd_palette_create) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_palette_to_gimp_palette", test_sd_palette_gimp_save) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_palette_from_gimp_palette", test_sd_palette_gimp_load) == NULL) { return; }
    if(CU_add_test(suite, "test of palette roundtripping", test_gimp_roundtrip) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_palette_free", test_sd_palette_free) == NULL) { return; }
}