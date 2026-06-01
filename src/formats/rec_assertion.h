/**
 * @file rec_assertion.h
 * @brief Match record assertion handling.
 * @details Functions and structs for parsing and formatting match record (REC) assertions.
 * @copyright MIT License
 * @date 2013-2026
 * @author OpenOMF Project
 */

#ifndef REC_ASSERTION_H
#define REC_ASSERTION_H

#include "utils/str.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Operator types
typedef enum
{
    OP_LT = 0,
    OP_GT,
    OP_EQ,
    OP_SET,
    OP_INVALID
} rec_assertion_operator;

// Object attributes
typedef enum
{
    ATTR_X_POS,
    ATTR_Y_POS,
    ATTR_X_VEL,
    ATTR_Y_VEL,
    ATTR_STATE_ID,
    ATTR_ANIMATION_ID,
    ATTR_HEALTH,
    ATTR_STAMINA,
    ATTR_OPPONENT_DISTANCE,
    ATTR_DIRECTION,
    ATTR_INVALID
} rec_har_attr;

// Operand structure
typedef struct {
    bool is_literal;
    union {
        struct {
            uint8_t har_id;
            rec_har_attr attribute;
        } attr;
        int16_t literal;
    } value;
} rec_assertion_operand;

// Full assertion structure
typedef struct {
    rec_assertion_operator op;
    rec_assertion_operand operand1;
    rec_assertion_operand operand2;
} rec_assertion;

/** @brief Parse an assertion from its binary encoding.
 *
 * @param data Source bytes holding the encoded assertion.
 * @param out Assertion struct to fill.
 * @return true on success, false if the data is invalid.
 */
bool parse_assertion(const uint8_t *data, rec_assertion *out);

/** @brief Encode an assertion to its binary form.
 *
 * @param assertion Assertion to encode.
 * @param buffer Destination buffer to write into.
 * @return true on success, false on failure.
 */
bool encode_assertion(const rec_assertion *assertion, uint8_t *buffer);

/** @brief Resolve a HAR attribute name to its enum value.
 *
 * @param key Attribute name.
 * @return The matching attribute, or ATTR_INVALID if not recognized.
 */
rec_har_attr rec_assertion_get_har_attr(const char *key);

/** @brief Build an assertion operand from textual operand and value.
 *
 * @param op Operand struct to fill.
 * @param operand Operand text (attribute name or literal).
 * @param value Value text.
 * @return Zero (SD_SUCCESS) on success, non-zero on failure.
 */
int rec_assertion_get_operand(rec_assertion_operand *op, const char *operand, const char *value);

/** @brief Format an assertion as a human-readable string.
 *
 * @param s Destination string.
 * @param assertion Assertion to format.
 */
void rec_assertion_to_str(str *s, const rec_assertion *assertion);

/** @brief Print an assertion to stdout.
 *
 * @param assertion Assertion to print.
 */
void print_assertion(const rec_assertion *assertion);

/** @brief Log an assertion at debug level.
 *
 * @param assertion Assertion to log.
 */
void log_assertion(const rec_assertion *assertion);

#endif // REC_ASSERTION_H
