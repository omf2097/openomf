/**
 * Tests for per-channel log filtering.
 *
 * Exercises channel-list parsing and enabled-channel selection.
 * Actual sink filtering is exercised via CLI-level checks.
 */

#include "utils/log.h"

#include <CUnit/CUnit.h>

void test_log_channels_single(void) {
    log_channel_filter ai = log_channels_from_string("ai");
    log_channel_filter rec = log_channels_from_string("rec");
    CU_ASSERT_TRUE(ai.active);
    CU_ASSERT_TRUE(ai.enabled[LOG_CHANNEL_AI]);
    CU_ASSERT_FALSE(ai.enabled[LOG_CHANNEL_REC]);
    CU_ASSERT_TRUE(rec.active);
    CU_ASSERT_TRUE(rec.enabled[LOG_CHANNEL_REC]);
    CU_ASSERT_FALSE(rec.enabled[LOG_CHANNEL_AI]);
}

void test_log_channels_multiple(void) {
    log_channel_filter ai_rec = log_channels_from_string("ai,rec");
    log_channel_filter rec_ai = log_channels_from_string("rec,ai");
    CU_ASSERT_TRUE(ai_rec.enabled[LOG_CHANNEL_AI]);
    CU_ASSERT_TRUE(ai_rec.enabled[LOG_CHANNEL_REC]);
    CU_ASSERT_FALSE(ai_rec.enabled[LOG_CHANNEL_NETWORK]);
    CU_ASSERT_TRUE(rec_ai.enabled[LOG_CHANNEL_AI]);
    CU_ASSERT_TRUE(rec_ai.enabled[LOG_CHANNEL_REC]);
}

void test_log_channels_whitespace(void) {
    log_channel_filter spaced = log_channels_from_string("ai, rec");
    log_channel_filter padded = log_channels_from_string(" rec ");
    CU_ASSERT_TRUE(spaced.enabled[LOG_CHANNEL_AI]);
    CU_ASSERT_TRUE(spaced.enabled[LOG_CHANNEL_REC]);
    CU_ASSERT_TRUE(padded.enabled[LOG_CHANNEL_REC]);
    CU_ASSERT_FALSE(padded.enabled[LOG_CHANNEL_AI]);
}

void test_log_channels_group(void) {
    log_channel_filter grouped = log_channels_from_string("ai,rec,ai");
    CU_ASSERT_TRUE(grouped.active);
    CU_ASSERT_TRUE(grouped.enabled[LOG_CHANNEL_AI]);
    CU_ASSERT_TRUE(grouped.enabled[LOG_CHANNEL_REC]);
}

void test_log_channels_empty_unset(void) {
    log_channel_filter empty = log_channels_from_string("");
    log_channel_filter unset = log_channels_from_string(NULL);
    log_channel_filter whitespace = log_channels_from_string(" , \t ");
    CU_ASSERT_FALSE(empty.active);
    CU_ASSERT_FALSE(unset.active);
    CU_ASSERT_FALSE(whitespace.active);
}

void test_log_channels_unknown_tokens_ignored(void) {
    log_channel_filter mixed = log_channels_from_string("typo,rec");
    log_channel_filter unknown = log_channels_from_string("typo");
    CU_ASSERT_TRUE(mixed.active);
    CU_ASSERT_TRUE(mixed.enabled[LOG_CHANNEL_REC]);
    CU_ASSERT_FALSE(mixed.enabled[LOG_CHANNEL_AI]);
    CU_ASSERT_FALSE(unknown.active);
}

void test_log_channel_values_are_distinct(void) {
    CU_ASSERT_TRUE(LOG_CHANNEL_AI != LOG_CHANNEL_REC);
    CU_ASSERT_TRUE(LOG_CHANNEL_AI != LOG_CHANNEL_NONE);
    CU_ASSERT_EQUAL(LOG_CHANNEL_NONE, 0);
    CU_ASSERT_EQUAL(LOG_CHANNEL_COUNT, LOG_CHANNEL_AUDIO + 1);
}

void log_channel_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "log channels: single tag", test_log_channels_single) == NULL)
        return;
    if(CU_add_test(suite, "log channels: multiple tags", test_log_channels_multiple) == NULL)
        return;
    if(CU_add_test(suite, "log channels: whitespace", test_log_channels_whitespace) == NULL)
        return;
    if(CU_add_test(suite, "log channels: group", test_log_channels_group) == NULL)
        return;
    if(CU_add_test(suite, "log channels: empty unset", test_log_channels_empty_unset) == NULL)
        return;
    if(CU_add_test(suite, "log channels: unknown tokens ignored", test_log_channels_unknown_tokens_ignored) == NULL)
        return;
    if(CU_add_test(suite, "log channels: values distinct", test_log_channel_values_are_distinct) == NULL)
        return;
}
