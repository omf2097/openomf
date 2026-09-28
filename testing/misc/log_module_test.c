/**
 * Tests for per-channel log filtering.
 *
 * Exercises the pure parsing logic log_channels_from_string() and the channel
 * bitmask constants. Actual sink filtering is exercised via CLI-level checks.
 */

#include "utils/log.h"

#include <CUnit/CUnit.h>

void test_log_channels_single(void) {
    CU_ASSERT_EQUAL(log_channels_from_string("ai"), LOG_CHANNEL_AI);
    CU_ASSERT_EQUAL(log_channels_from_string("ai-state"), LOG_CHANNEL_AI_STATE);
    CU_ASSERT_EQUAL(log_channels_from_string("ai-decision"), LOG_CHANNEL_AI_DECISION);
    CU_ASSERT_EQUAL(log_channels_from_string("rec"), LOG_CHANNEL_REC);
}

void test_log_channels_multiple(void) {
    CU_ASSERT_EQUAL(log_channels_from_string("ai-state,ai-decision"),
                    LOG_CHANNEL_AI_STATE | LOG_CHANNEL_AI_DECISION);
    CU_ASSERT_EQUAL(log_channels_from_string("ai,rec"), LOG_CHANNEL_AI | LOG_CHANNEL_REC);
    CU_ASSERT_EQUAL(log_channels_from_string("rec,ai-state"),
                    LOG_CHANNEL_REC | LOG_CHANNEL_AI_STATE);
}

void test_log_channels_whitespace(void) {
    CU_ASSERT_EQUAL(log_channels_from_string("ai-state, ai-decision"),
                    LOG_CHANNEL_AI_STATE | LOG_CHANNEL_AI_DECISION);
    CU_ASSERT_EQUAL(log_channels_from_string(" rec "), LOG_CHANNEL_REC);
}

void test_log_channels_group(void) {
    CU_ASSERT_EQUAL(log_channels_from_string("ai"), LOG_CHANNEL_AI);
    CU_ASSERT_EQUAL(log_channels_from_string("ai,rec"), LOG_CHANNEL_AI | LOG_CHANNEL_REC);
    CU_ASSERT_EQUAL(log_channels_from_string("ai-state,ai"), LOG_CHANNEL_AI);
}

void test_log_channels_empty_unset(void) {
    CU_ASSERT_EQUAL(log_channels_from_string(""), LOG_CHANNEL_NONE);
    CU_ASSERT_EQUAL(log_channels_from_string(NULL), LOG_CHANNEL_NONE);
}

void test_log_channels_unknown_tokens_ignored(void) {
    CU_ASSERT_EQUAL(log_channels_from_string("typo,rec"), LOG_CHANNEL_REC);
    CU_ASSERT_EQUAL(log_channels_from_string("typo"), LOG_CHANNEL_NONE);
}

void test_log_channel_bits_are_distinct(void) {
    CU_ASSERT_TRUE(LOG_CHANNEL_AI_STATE != LOG_CHANNEL_AI_DECISION);
    CU_ASSERT_TRUE(LOG_CHANNEL_AI_DECISION != LOG_CHANNEL_REC);
    CU_ASSERT_EQUAL(LOG_CHANNEL_NONE, 0);
}

void log_channel_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "log channels: single tag", test_log_channels_single) == NULL) return;
    if(CU_add_test(suite, "log channels: multiple tags", test_log_channels_multiple) == NULL) return;
    if(CU_add_test(suite, "log channels: whitespace", test_log_channels_whitespace) == NULL) return;
    if(CU_add_test(suite, "log channels: group", test_log_channels_group) == NULL) return;
    if(CU_add_test(suite, "log channels: empty unset", test_log_channels_empty_unset) == NULL) return;
    if(CU_add_test(suite, "log channels: unknown tokens ignored", test_log_channels_unknown_tokens_ignored) == NULL)
        return;
    if(CU_add_test(suite, "log channels: bits distinct", test_log_channel_bits_are_distinct) == NULL) return;
}
