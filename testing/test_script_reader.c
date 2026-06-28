#include "common.h"
#include "formats/script.h"
#include "formats/script_reader.h"

#define READER_STR "s05bpd1bps1bpn64A100-s1sf3B10-C34"

void test_script_reader_basic(void) {
    script s;
    script_create(&s);
    script_decode(&s, READER_STR, NULL);

    script_reader r;
    script_reader_load(&r, &s);

    script_reader_seek(&r, 0);
    CU_ASSERT_PTR_NOT_NULL(script_reader_frame(&r));
    CU_ASSERT(script_reader_isset(&r, TAG_S) == true);
    CU_ASSERT(script_reader_get(&r, TAG_S) == 5);
    CU_ASSERT(script_reader_get(&r, TAG_BPN) == 64);
    CU_ASSERT(script_reader_isset(&r, TAG_SF) == false);
    CU_ASSERT(script_reader_get(&r, TAG_SF) == 0);

    CU_ASSERT(script_get_total_ticks(script_reader_get_script(&r)) == 144);

    script_free(&s);
}

void test_script_reader_seek(void) {
    script s;
    script_create(&s);
    script_decode(&s, READER_STR, NULL);

    script_reader r;
    script_reader_load(&r, &s);

    // Frame 1.
    script_reader_seek(&r, 100);
    CU_ASSERT(script_reader_get(&r, TAG_S) == 1);
    CU_ASSERT(script_reader_get(&r, TAG_SF) == 3);
    CU_ASSERT(script_reader_isset(&r, TAG_BPD) == false);

    // Frame 2 has no tags.
    script_reader_seek(&r, 110);
    CU_ASSERT_PTR_NOT_NULL(script_reader_frame(&r));
    CU_ASSERT(script_reader_isset(&r, TAG_S) == false);

    // Last in-range tick still resolves to frame 2.
    script_reader_seek(&r, 143);
    CU_ASSERT_PTR_NOT_NULL(script_reader_frame(&r));

    // Past the end: no frame.
    script_reader_seek(&r, 144);
    CU_ASSERT_PTR_NULL(script_reader_frame(&r));
    CU_ASSERT(script_reader_isset(&r, TAG_S) == false);
    CU_ASSERT(script_reader_get(&r, TAG_S) == 0);

    script_free(&s);
}

void test_script_reader_cache(void) {
    script s;
    script_create(&s);
    script_decode(&s, READER_STR, NULL);

    script_reader r;
    script_reader_load(&r, &s);

    // Two ticks inside the same frame resolve to the same frame object.
    script_reader_seek(&r, 0);
    const script_frame *f0 = script_reader_frame(&r);
    script_reader_seek(&r, 50);
    CU_ASSERT(script_reader_frame(&r) == f0);

    // A tick in a different frame resolves to a different frame.
    script_reader_seek(&r, 100);
    const script_frame *f1 = script_reader_frame(&r);
    CU_ASSERT(f1 != f0);
    CU_ASSERT_PTR_NOT_NULL(f1);

    // Seeking back returns the original frame again.
    script_reader_seek(&r, 10);
    CU_ASSERT(script_reader_frame(&r) == f0);

    script_free(&s);
}

void test_script_reader_advance(void) {
    script s;
    script_create(&s);
    script_decode(&s, READER_STR, NULL);

    script_reader r;
    script_reader_load(&r, &s);

    // Step across a frame boundary one tick at a time (forward fast path).
    script_reader_seek(&r, 99);
    CU_ASSERT(script_reader_isset(&r, TAG_BPN) == true); // still frame 0
    script_reader_advance(&r, 1);
    CU_ASSERT(script_reader_tick(&r) == 100);
    CU_ASSERT(script_reader_get(&r, TAG_SF) == 3); // now frame 1
    CU_ASSERT(script_reader_isset(&r, TAG_BPN) == false);

    script_free(&s);
}

void test_script_reader_decode_invalidates(void) {
    script s1, s2;
    script_create(&s1);
    script_decode(&s1, READER_STR, NULL);
    script_create(&s2);
    script_decode(&s2, "A10-", NULL);

    script_reader r;

    script_reader_load(&r, &s1);
    script_reader_seek(&r, 0);
    CU_ASSERT(script_reader_isset(&r, TAG_S) == true);

    // Loading another script repoints the reader, resets to tick 0, and drops the stale cache.
    script_reader_load(&r, &s2);
    CU_ASSERT(script_reader_tick(&r) == 0);
    CU_ASSERT_PTR_NOT_NULL(script_reader_frame(&r));
    CU_ASSERT(script_reader_isset(&r, TAG_S) == false); // new script has no tags
    CU_ASSERT(script_get_total_ticks(script_reader_get_script(&r)) == 10);

    script_free(&s1);
    script_free(&s2);
}

void script_reader_test_suite(CU_pSuite suite) {
    ADD_TEST("test of script_reader basics", test_script_reader_basic);
    ADD_TEST("test of script_reader seek", test_script_reader_seek);
    ADD_TEST("test of script_reader frame cache", test_script_reader_cache);
    ADD_TEST("test of script_reader advance", test_script_reader_advance);
    ADD_TEST("test of script_reader decode invalidation", test_script_reader_decode_invalidates);
}
