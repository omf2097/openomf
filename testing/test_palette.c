#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <shadowdive/shadowdive.h>

#define TESTFILE "test.gpl"

void test_gimp_roundtrip(void) {
    sd_palette pal;
    sd_palette new;
    int ret;

    // Create
    ret = sd_palette_create(&pal);
    CU_ASSERT(ret == SD_SUCCESS);
    ret = sd_palette_create(&new);
    CU_ASSERT(ret == SD_SUCCESS);

    // Fill with data
    for(int i = 0; i < 256; i++) {
        pal.data[i][0] = i;
        pal.data[i][1] = 256-i;
        pal.data[i][2] = 128;
    }

    // Write
    ret = sd_palette_to_gimp_palette(&pal, TESTFILE);
    CU_ASSERT(ret == SD_SUCCESS);

    // Read
    ret = sd_palette_from_gimp_palette(&new, TESTFILE);
    CU_ASSERT(ret == SD_SUCCESS);

    // Match
    CU_ASSERT_NSTRING_EQUAL(pal.data, new.data, 256*3);

    // Free up
    sd_palette_free(&pal);
    sd_palette_free(&new);
}

void palette_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "test of palette roundtripping", test_gimp_roundtrip) == NULL) { return; }
}