#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <shadowdive/shadowdive.h>
#include "misc/parser_test_strings.h"

sd_script script;

#define OK_STR "bpd1bps1bpn64A100-s1sf3B10-C34"

void test_script_create(void) {
    CU_ASSERT(sd_script_create(&script) == SD_SUCCESS);
    CU_ASSERT(sd_script_create(NULL) == SD_INVALID_INPUT);
    CU_ASSERT(script.frame_count == 0);
}

void test_script_decode(void) {
    CU_ASSERT(sd_script_decode(&script, NULL, NULL) == SD_INVALID_INPUT);
    CU_ASSERT(sd_script_decode(NULL, OK_STR, NULL) == SD_INVALID_INPUT);
    CU_ASSERT(sd_script_decode(&script, OK_STR, NULL) == SD_SUCCESS);
    CU_ASSERT(script.frame_count == 3);
    CU_ASSERT(script.frames[0].tag_count == 3);
    CU_ASSERT(script.frames[1].tag_count == 2);
    CU_ASSERT(script.frames[2].tag_count == 0);
}

void test_script_encode(void) {
    char buf[1024];
    memset(buf, 0, 1024);
    CU_ASSERT(sd_script_encode(&script, buf) == SD_SUCCESS);
    CU_ASSERT(strcmp(OK_STR, buf) == 0);
}

void test_script_encoded_length(void) {
    int len = sd_script_encoded_length(&script);
    CU_ASSERT(len = strlen(OK_STR));
}

void test_script_all(void) {
    char buf[1024];
    for(int i = 0; i < TEST_STRING_COUNT; i++) {
        sd_script s;
        CU_ASSERT_FATAL(sd_script_create(&s) == SD_SUCCESS);
        int ret = sd_script_decode(&s, test_strings[i], NULL);
        if(ret == SD_SUCCESS) {
            CU_ASSERT(sd_script_encode(&s, buf) == SD_SUCCESS);
        } else {
            CU_FAIL("Parser failed. Broken string ?");
        }
        sd_script_free(&s);
    }
}

void test_script_free(void) {
    sd_script_free(&script);
}

void script_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "test of sd_script_create", test_script_create) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_decode", test_script_decode) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_encoded_length", test_script_encoded_length) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_encode", test_script_encode) == NULL) { return; }
    if(CU_add_test(suite, "test of all OMF strings", test_script_all) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_script_free", test_script_free) == NULL) { return; }
}