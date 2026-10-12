#include "formats/rec_assertion.h"
#include "CUnit/CUnit.h"
#include <string.h>

static void test_rec_assertion_roundtrip(void) {
    rec_assertion input = {
        .op = OP_EQ,
        .operand1 = {
            .is_literal = false,
            .value.attr = {.har_id = 0, .attribute = ATTR_ANIMATION_ID},
        },
        .operand2 = {
            .is_literal = true,
            .value.literal = 12,
        },
    };
    uint8_t encoded[8] = {0};
    rec_assertion decoded;

    CU_ASSERT_TRUE(encode_assertion(&input, encoded));
    CU_ASSERT_TRUE(parse_assertion(encoded, &decoded));
    CU_ASSERT_EQUAL(decoded.op, input.op);
    CU_ASSERT_FALSE(decoded.operand1.is_literal);
    CU_ASSERT_EQUAL(decoded.operand1.value.attr.har_id, 0);
    CU_ASSERT_EQUAL(decoded.operand1.value.attr.attribute, ATTR_ANIMATION_ID);
    CU_ASSERT_TRUE(decoded.operand2.is_literal);
    CU_ASSERT_EQUAL(decoded.operand2.value.literal, 12);
}

static void test_rec_assertion_set_and_attribute_names(void) {
    rec_assertion input = {
        .op = OP_SET,
        .operand1 = {
            .is_literal = false,
            .value.attr = {.har_id = 1, .attribute = ATTR_X_POS},
        },
        .operand2 = {
            .is_literal = true,
            .value.literal = 300,
        },
    };
    uint8_t encoded[8] = {0};
    rec_assertion decoded;

    CU_ASSERT_TRUE(encode_assertion(&input, encoded));
    CU_ASSERT_TRUE(parse_assertion(encoded, &decoded));
    CU_ASSERT_EQUAL(decoded.op, OP_SET);
    CU_ASSERT_EQUAL(decoded.operand1.value.attr.har_id, 1);
    CU_ASSERT_EQUAL(decoded.operand1.value.attr.attribute, ATTR_X_POS);
    CU_ASSERT_EQUAL(decoded.operand2.value.literal, 300);
    CU_ASSERT_EQUAL(rec_assertion_get_har_attr("health"), ATTR_HEALTH);
    CU_ASSERT_EQUAL(rec_assertion_get_har_attr("not_an_attribute"), ATTR_INVALID);
}

static void test_rec_assertion_rejects_invalid_data(void) {
    uint8_t invalid[8] = {0};
    rec_assertion assertion;

    CU_ASSERT_FALSE(parse_assertion(invalid, &assertion));
}

void rec_assertion_test_suite(CU_pSuite suite) {
    CU_add_test(suite, "assertion encode/decode", test_rec_assertion_roundtrip);
    CU_add_test(suite, "set assertion and attribute names", test_rec_assertion_set_and_attribute_names);
    CU_add_test(suite, "reject invalid assertion data", test_rec_assertion_rejects_invalid_data);
}
