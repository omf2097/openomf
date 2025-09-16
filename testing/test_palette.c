#include "formats/error.h"
#include "formats/palette.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdio.h>

#define TESTFILE "test.gpl"
#define TESTFILE2 "test2.gpl"

static path tmp_dir;

vga_palette pal;

void test_palette_create(void) {
    // Fill with data
    for(int i = 0; i < 256; i++) {
        pal.colors[i].r = i;
        pal.colors[i].g = 256 - i;
        pal.colors[i].b = 128;
    }
}

void test_palette_gimp_save(void) {
    int ret;
    path test_file = tmp_dir;
    path_append(&test_file, TESTFILE);

    ret = palette_to_gimp_palette(&pal, NULL);
    CU_ASSERT(ret == SD_INVALID_INPUT);
    ret = palette_to_gimp_palette(NULL, &test_file);
    CU_ASSERT(ret == SD_INVALID_INPUT);
    ret = palette_to_gimp_palette(&pal, &test_file);
    CU_ASSERT(ret == SD_SUCCESS);
}

void test_palette_gimp_load(void) {
    int ret;
    path test_file, test_file2;
    test_file = test_file2 = tmp_dir;
    path_append(&test_file, TESTFILE);
    path_append(&test_file2, TESTFILE2);

    ret = palette_from_gimp_palette(&pal, NULL);
    CU_ASSERT(ret == SD_INVALID_INPUT);
    ret = palette_from_gimp_palette(NULL, &test_file);
    CU_ASSERT(ret == SD_INVALID_INPUT);

    path nonexistent;
    path_from_c(&nonexistent, "nonesuchfile.gpl");
    ret = palette_from_gimp_palette(&pal, &nonexistent);
    CU_ASSERT(ret == SD_FILE_OPEN_ERROR);

    // Create an invalid file
    FILE *tmp = path_fopen(&test_file2, "wb");
    CU_ASSERT(tmp != NULL);
    fclose(tmp);

    // Test invalid file
    ret = palette_from_gimp_palette(&pal, &test_file2);
    CU_ASSERT(ret == SD_FILE_INVALID_TYPE);

    // Test good file
    ret = palette_from_gimp_palette(&pal, &test_file);
    CU_ASSERT(ret == SD_SUCCESS);
}

void test_gimp_roundtrip(void) {
    vga_palette new;
    vga_palette_init(&new);
    int ret;
    path test_file = tmp_dir;
    path_append(&test_file, TESTFILE);

    // Write
    ret = palette_to_gimp_palette(&pal, &test_file);
    CU_ASSERT(ret == SD_SUCCESS);

    // Read
    ret = palette_from_gimp_palette(&new, &test_file);
    CU_ASSERT(ret == SD_SUCCESS);

    // Match
    CU_ASSERT_NSTRING_EQUAL(pal.colors, new.colors, 256 * 3);
}

void palette_test_suite(CU_pSuite suite) {
    path_create_tmpdir(&tmp_dir);
    if(CU_add_test(suite, "test of palette_create", test_palette_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of palette_to_gimp_palette", test_palette_gimp_save) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of palette_from_gimp_palette", test_palette_gimp_load) == NULL) {
        return;
    }
    if(CU_add_test(suite, "test of palette roundtripping", test_gimp_roundtrip) == NULL) {
        return;
    }
}
