#include "common.h"
#include "utils/sstream.h"

void test_sstream_basic(void) {
    sstream s;
    sstream_open_c(&s, "abc");

    CU_ASSERT(sstream_pos(&s) == 0);
    CU_ASSERT(sstream_left(&s) == 3);
    CU_ASSERT(!sstream_eof(&s));
    CU_ASSERT(sstream_peek(&s) == 'a');
    CU_ASSERT(*sstream_ptr(&s) == 'a');
    CU_ASSERT(sstream_get(&s) == 'a');
    CU_ASSERT(sstream_get(&s) == 'b');
    CU_ASSERT(sstream_pos(&s) == 2);
    CU_ASSERT(sstream_left(&s) == 1);
    CU_ASSERT(sstream_get(&s) == 'c');

    // EOF
    CU_ASSERT(sstream_eof(&s));
    CU_ASSERT(sstream_left(&s) == 0);
    CU_ASSERT(sstream_peek(&s) == '\0');
    CU_ASSERT(sstream_get(&s) == '\0');
    CU_ASSERT(sstream_pos(&s) == 3);
}

void test_sstream_peek_at_and_skip(void) {
    sstream s;
    sstream_open_c(&s, "hello");
    CU_ASSERT(sstream_peek_at(&s, 0) == 'h');
    CU_ASSERT(sstream_peek_at(&s, 4) == 'o');
    CU_ASSERT(sstream_peek_at(&s, 5) == '\0');
    CU_ASSERT(sstream_peek_at(&s, -1) == '\0');

    // Make sure peek works
    sstream_skip(&s, 2);
    CU_ASSERT(sstream_pos(&s) == 2);
    CU_ASSERT(sstream_peek(&s) == 'l');
    CU_ASSERT(sstream_peek_at(&s, -2) == 'h');

    // Positive skip clamps to len
    sstream_skip(&s, 100);
    CU_ASSERT(sstream_pos(&s) == 5);
    CU_ASSERT(sstream_eof(&s));

    // Negative skip clamps to 0
    sstream_skip(&s, -100);
    CU_ASSERT(sstream_pos(&s) == 0);
}

void test_sstream_read_long(void) {
    sstream s;
    long v;

    sstream_open_c(&s, "123");
    CU_ASSERT(sstream_read_long(&s, &v, INT16_MIN, INT16_MAX) == true);
    CU_ASSERT(v == 123);
    CU_ASSERT(sstream_eof(&s));

    // Negative sign
    sstream_open_c(&s, "-42");
    CU_ASSERT(sstream_read_long(&s, &v, INT16_MIN, INT16_MAX) == true);
    CU_ASSERT(v == -42);

    // Positive sign
    sstream_open_c(&s, "+7");
    CU_ASSERT(sstream_read_long(&s, &v, INT16_MIN, INT16_MAX) == true);
    CU_ASSERT(v == 7);

    // Properly handle leading zeroes
    sstream_open_c(&s, "05");
    CU_ASSERT(sstream_read_long(&s, &v, INT16_MIN, INT16_MAX) == true);
    CU_ASSERT(v == 5);

    // Large value but no clamping
    sstream_open_c(&s, "20000");
    CU_ASSERT(sstream_read_long(&s, &v, INT16_MIN, INT16_MAX) == true);
    CU_ASSERT(v == 20000);

    // Long value with clamping
    sstream_open_c(&s, "70000");
    CU_ASSERT(sstream_read_long(&s, &v, INT16_MIN, INT16_MAX) == true);
    CU_ASSERT(v == INT16_MAX);
    CU_ASSERT(sstream_eof(&s));

    // Should stop when no more digits
    sstream_open_c(&s, "12a");
    CU_ASSERT(sstream_read_long(&s, &v, INT16_MIN, INT16_MAX) == true);
    CU_ASSERT(v == 12);
    CU_ASSERT(sstream_peek(&s) == 'a');

    // No number
    sstream_open_c(&s, "x9");
    CU_ASSERT(sstream_read_long(&s, &v, INT16_MIN, INT16_MAX) == false);
    CU_ASSERT(v == 0);
    CU_ASSERT(sstream_pos(&s) == 0);
}

void sstream_test_suite(CU_pSuite suite) {
    ADD_TEST("test of sstream basics", test_sstream_basic);
    ADD_TEST("test of sstream_peek_at and sstream_skip", test_sstream_peek_at_and_skip);
    ADD_TEST("test of sstream_read_long", test_sstream_read_long);
}
