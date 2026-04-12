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
    for(int i = 0; i < VGA_PALETTE_SIZE; i++) {
        pal.colors[i].r = i % 256;
        pal.colors[i].g = (256 - i) % 256;
        pal.colors[i].b = 128;
    }
}

void test_palette_gimp_save(void) {
    path test_file = tmp_dir;
    path_append(&test_file, TESTFILE);
    CU_ASSERT(palette_to_gimp_palette(&pal, NULL) == SD_INVALID_INPUT);
    CU_ASSERT(palette_to_gimp_palette(NULL, &test_file) == SD_INVALID_INPUT);
    CU_ASSERT(palette_to_gimp_palette(&pal, &test_file) == SD_SUCCESS);
}

void test_palette_gimp_load(void) {
    path test_file, test_file2;
    test_file = test_file2 = tmp_dir;
    path_append(&test_file, TESTFILE);
    path_append(&test_file2, TESTFILE2);

    CU_ASSERT(palette_from_gimp_palette(&pal, NULL) == SD_INVALID_INPUT);
    CU_ASSERT(palette_from_gimp_palette(NULL, &test_file) == SD_INVALID_INPUT);

    path nonexistent;
    path_from_c(&nonexistent, "nonesuchfile.gpl");
    CU_ASSERT(palette_from_gimp_palette(&pal, &nonexistent) == SD_FILE_OPEN_ERROR);

    // Create an invalid file
    FILE *tmp = path_fopen(&test_file2, "wb");
    CU_ASSERT(tmp != NULL);
    fclose(tmp);

    // Test invalid file
    CU_ASSERT(palette_from_gimp_palette(&pal, &test_file2) == SD_FILE_INVALID_TYPE);

    // Test good file
    CU_ASSERT(palette_from_gimp_palette(&pal, &test_file) == SD_SUCCESS);
}

void test_gimp_roundtrip(void) {
    vga_palette new;
    vga_palette_init(&new);
    path test_file = tmp_dir;
    path_append(&test_file, TESTFILE);

    // Write
    CU_ASSERT(palette_to_gimp_palette(&pal, &test_file) == SD_SUCCESS);

    // Read
    CU_ASSERT(palette_from_gimp_palette(&new, &test_file) == SD_SUCCESS);

    // Match
    CU_ASSERT_NSTRING_EQUAL(pal.colors, new.colors, VGA_PALETTE_SIZE * 3);
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
