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

bool parse_assertion(const uint8_t *data, rec_assertion *out);

bool encode_assertion(const rec_assertion *assertion, uint8_t *buffer);

rec_har_attr rec_assertion_get_har_attr(const char *key);
int rec_assertion_get_operand(rec_assertion_operand *op, const char *operand, const char *value);

void rec_assertion_to_str(str *s, const rec_assertion *assertion);
void print_assertion(const rec_assertion *assertion);
void log_assertion(const rec_assertion *assertion);

#endif // REC_ASSERTION_H
