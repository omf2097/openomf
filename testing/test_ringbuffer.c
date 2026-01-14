#include "utils/ringbuffer.h"
#include <CUnit/CUnit.h>
#include <string.h>

void test_rb_create(void) {
    ring_buffer rb;
    rb_create(&rb, 64);

    CU_ASSERT_EQUAL(rb_size(&rb), 64);
    CU_ASSERT_EQUAL(rb_length(&rb), 0);

    rb_free(&rb);
}

void test_rb_write_read_simple(void) {
    ring_buffer rb;
    rb_create(&rb, 64);

    const char *data = "Cthulhu fhtagn!";
    const size_t len = strlen(data);

    size_t written = rb_write(&rb, data, len);
    CU_ASSERT_EQUAL(written, len);
    CU_ASSERT_EQUAL(rb_length(&rb), len);

    char buffer[64] = {0};
    size_t read = rb_read(&rb, buffer, len);
    CU_ASSERT_EQUAL(read, len);
    CU_ASSERT_EQUAL(rb_length(&rb), 0);
    CU_ASSERT_STRING_EQUAL(buffer, data);

    rb_free(&rb);
}

void test_rb_write_partial(void) {
    ring_buffer rb;
    rb_create(&rb, 8);

    const char *data = "1234567890ABCDEF"; // 16 bytes, only 8 fit
    CU_ASSERT_EQUAL(rb_write(&rb, data, 16), 8);
    CU_ASSERT_EQUAL(rb_length(&rb), 8);

    char buffer[16] = {0};
    CU_ASSERT_EQUAL(rb_read(&rb, buffer, 16), 8);
    CU_ASSERT(memcmp(buffer, "12345678", 8) == 0);

    rb_free(&rb);
}

void test_rb_write_when_full(void) {
    ring_buffer rb;
    rb_create(&rb, 8);

    const char *data = "12345678";
    CU_ASSERT_EQUAL(rb_write(&rb, data, 8), 8);
    CU_ASSERT_EQUAL(rb_length(&rb), 8);

    // buffer full, should not write
    CU_ASSERT_EQUAL(rb_write(&rb, "more", 4), 0);
    CU_ASSERT_EQUAL(rb_length(&rb), 8);

    rb_free(&rb);
}

void test_rb_peek(void) {
    ring_buffer rb;
    rb_create(&rb, 64);

    const char *data = "TestData";
    rb_write(&rb, data, 8);

    char buffer[16] = {0};
    CU_ASSERT_EQUAL(rb_peek(&rb, buffer, 8), 8);
    CU_ASSERT_EQUAL(rb_length(&rb), 8); // Length unchanged after peek
    CU_ASSERT(memcmp(buffer, data, 8) == 0);

    // Peek again, should get the same data
    memset(buffer, 0, sizeof(buffer));
    CU_ASSERT_EQUAL(rb_peek(&rb, buffer, 8), 8);
    CU_ASSERT(memcmp(buffer, data, 8) == 0);

    rb_free(&rb);
}

void test_rb_skip(void) {
    ring_buffer rb;
    rb_create(&rb, 64);

    const char *data = "0123456789";
    rb_write(&rb, data, 10);
    CU_ASSERT_EQUAL(rb_length(&rb), 10);

    CU_ASSERT_EQUAL(rb_skip(&rb, 5), 5);
    CU_ASSERT_EQUAL(rb_length(&rb), 5);

    char buffer[16] = {0};
    CU_ASSERT_EQUAL(rb_read(&rb, buffer, 5), 5);
    CU_ASSERT(memcmp(buffer, "56789", 5) == 0);

    rb_free(&rb);
}

void test_rb_wraparound(void) {
    ring_buffer rb;
    rb_create(&rb, 8);
    rb_write(&rb, "ABCDEF", 6);
    CU_ASSERT_EQUAL(rb_length(&rb), 6);

    // read 4, leaving 2
    char buffer[16] = {0};
    CU_ASSERT_EQUAL(rb_read(&rb, buffer, 4), 4);
    CU_ASSERT_EQUAL(rb_length(&rb), 2);
    CU_ASSERT(memcmp(buffer, "ABCD", 4) == 0);

    // write 5, wraps
    CU_ASSERT_EQUAL(rb_write(&rb, "12345", 5), 5);
    CU_ASSERT_EQUAL(rb_length(&rb), 7);

    // read all
    memset(buffer, 0, sizeof(buffer));
    CU_ASSERT_EQUAL(rb_read(&rb, buffer, 7), 7);
    CU_ASSERT(memcmp(buffer, "EF12345", 7) == 0);

    rb_free(&rb);
}

void test_rb_read_empty(void) {
    ring_buffer rb;
    rb_create(&rb, 8);

    char buffer[8] = {0};
    CU_ASSERT_EQUAL(rb_read(&rb, buffer, 8), 0);

    rb_free(&rb);
}

void test_rb_skip_empty(void) {
    ring_buffer rb;
    rb_create(&rb, 8);

    CU_ASSERT_EQUAL(rb_skip(&rb, 5), 0);

    rb_free(&rb);
}

void test_rb_peek_empty(void) {
    ring_buffer rb;
    rb_create(&rb, 8);

    char buffer[8] = {0};
    CU_ASSERT_EQUAL(rb_peek(&rb, buffer, 8), 0);

    rb_free(&rb);
}

void ringbuffer_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "Test ringbuffer create", test_rb_create) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test ringbuffer write and read", test_rb_write_read_simple) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test ringbuffer partial write", test_rb_write_partial) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test ringbuffer write when full", test_rb_write_when_full) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test ringbuffer peek", test_rb_peek) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test ringbuffer skip", test_rb_skip) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test ringbuffer wraparound", test_rb_wraparound) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test ringbuffer read empty", test_rb_read_empty) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test ringbuffer skip empty", test_rb_skip_empty) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test ringbuffer peek empty", test_rb_peek_empty) == NULL) {
        return;
    }
}
