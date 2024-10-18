#include "formats/error.h"
#include "formats/palette.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdio.h>

#define TESTFILE "test.gpl"
#define TESTFILE2 "test2.gpl"

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

    ret = palette_to_gimp_palette(&pal, NULL);
    CU_ASSERT(ret == SD_INVALID_INPUT);
    ret = palette_to_gimp_palette(NULL, TESTFILE);
    CU_ASSERT(ret == SD_INVALID_INPUT);
    ret = palette_to_gimp_palette(&pal, TESTFILE);
    CU_ASSERT(ret == SD_SUCCESS);
}

void test_palette_gimp_load(void) {
    int ret;
    FILE *tmp;

    ret = palette_from_gimp_palette(&pal, NULL);
    CU_ASSERT(ret == SD_INVALID_INPUT);
    ret = palette_from_gimp_palette(NULL, TESTFILE);
    CU_ASSERT(ret == SD_INVALID_INPUT);

    ret = palette_from_gimp_palette(&pal, "nonesuchfile.gpl");
    CU_ASSERT(ret == SD_FILE_OPEN_ERROR);

    // Create an invalid file
    tmp = fopen(TESTFILE2, "wb");
    CU_ASSERT(tmp != NULL);
    fclose(tmp);

    // Test invalid file
    ret = palette_from_gimp_palette(&pal, TESTFILE2);
    CU_ASSERT(ret == SD_FILE_INVALID_TYPE);

    // Test good file
    ret = palette_from_gimp_palette(&pal, TESTFILE);
    CU_ASSERT(ret == SD_SUCCESS);
}

void test_gimp_roundtrip(void) {
    vga_palette new;
    vga_palette_init(&new);
    int ret;

    // Write
    ret = palette_to_gimp_palette(&pal, TESTFILE);
    CU_ASSERT(ret == SD_SUCCESS);

    // Read
    ret = palette_from_gimp_palette(&new, TESTFILE);
    CU_ASSERT(ret == SD_SUCCESS);

    // Match
    CU_ASSERT_NSTRING_EQUAL(pal.colors, new.colors, 256 * 3);
}

void palette_test_suite(CU_pSuite suite) {
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
