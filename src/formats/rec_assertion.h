#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Operator types
typedef enum
{
    OP_LT = 0,
    OP_GT,
    OP_EQ,
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
        uint16_t literal;
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

void print_assertion(const rec_assertion *assertion);
