

#include "formats/rec_assertion.h"
#include "formats/rec.h"
#include "utils/log.h"
#include "utils/str.h"

// Bit reader helper structure
typedef struct {
    const uint8_t *data;
    size_t bit_position;
} bit_reader;

// Read bits from the buffer
static uint32_t read_bits(bit_reader *reader, uint8_t num_bits) {
    uint32_t result = 0;
    for(uint8_t i = 0; i < num_bits; i++) {
        size_t byte_pos = reader->bit_position / 8;
        uint8_t bit_pos = 7 - (reader->bit_position % 8);
        uint8_t bit = (reader->data[byte_pos] >> bit_pos) & 1;
        result = (result << 1) | bit;
        reader->bit_position++;
    }
    return result;
}

// Bit writer helper structure
typedef struct {
    uint8_t *buffer;
    size_t bit_position;
} bit_writer;

// Write bits to buffer (MSB-first)
static void write_bits(bit_writer *writer, uint32_t value, uint8_t num_bits) {
    for(int8_t i = num_bits - 1; i >= 0; i--) {
        if(writer->bit_position >= 64)
            return; // Don't overflow 8-byte buffer

        uint8_t bit = (value >> i) & 1;
        size_t byte_pos = writer->bit_position / 8;
        uint8_t bit_pos = 7 - (writer->bit_position % 8);

        if(bit) {
            writer->buffer[byte_pos] |= (1 << bit_pos);
        }
        writer->bit_position++;
    }
}

// Parse the binary assertion
bool parse_assertion(const uint8_t *data, rec_assertion *out) {
    bit_reader reader = {data, 0};

    // packet 10 is differentiated by its first byte.
    if(read_bits(&reader, 8) != REC_LOOKUP10_ASSERT_BYTE) {
        return false;
    }

    // Read header
    uint8_t op_code = read_bits(&reader, 3);
    uint8_t op1_type = read_bits(&reader, 1);
    uint8_t op2_type = read_bits(&reader, 1);

    // Map operator
    switch(op_code) {
        case OP_LT:
            out->op = OP_LT;
            break;
        case OP_GT:
            out->op = OP_GT;
            break;
        case OP_EQ:
            out->op = OP_EQ;
            break;
        case OP_SET:
            out->op = OP_SET;
            break;
        default:
            return false;
    }

    // Parse operand 1
    out->operand1.is_literal = op1_type;
    if(op1_type) { // Literal
        out->operand1.value.literal = read_bits(&reader, 16);
    } else { // Object attribute
        out->operand1.value.attr.har_id = (int16_t)read_bits(&reader, 1);
        uint8_t attr = read_bits(&reader, 8);
        if(attr >= ATTR_INVALID)
            return false;
        out->operand1.value.attr.attribute = (rec_har_attr)attr;
    }

    // Parse operand 2
    out->operand2.is_literal = op2_type;
    if(op2_type) { // Literal
        out->operand2.value.literal = (int16_t)read_bits(&reader, 16);
    } else { // Object attribute
        out->operand2.value.attr.har_id = read_bits(&reader, 1);
        uint8_t attr = read_bits(&reader, 8);
        if(attr >= ATTR_INVALID)
            return false;
        out->operand2.value.attr.attribute = (rec_har_attr)attr;
    }

    return true;
}

bool encode_assertion(const rec_assertion *assertion, uint8_t *buffer) {
    // Validate operator
    if(assertion->op < 0 || assertion->op >= OP_INVALID)
        return false;

    // Validate operands
    const rec_assertion_operand *ops[2] = {&assertion->operand1, &assertion->operand2};
    for(int i = 0; i < 2; i++) {
        if(!ops[i]->is_literal) { // Validate attributes
            if(ops[i]->value.attr.har_id > 1)
                return false;
            if(ops[i]->value.attr.attribute >= ATTR_INVALID)
                return false;
        }
    }

    // Initialize buffer to zeros
    memset(buffer, 0, 8);
    bit_writer writer = {buffer, 0};

    // write packet sub-type for an Assert.
    write_bits(&writer, REC_LOOKUP10_ASSERT_BYTE, 8);

    // Write header (3b op + 1b types)
    write_bits(&writer, assertion->op, 3);
    write_bits(&writer, assertion->operand1.is_literal, 1);
    write_bits(&writer, assertion->operand2.is_literal, 1);

    // Write operands
    for(int i = 0; i < 2; i++) {
        if(ops[i]->is_literal) {
            write_bits(&writer, (uint16_t)ops[i]->value.literal, 16);
        } else {
            write_bits(&writer, ops[i]->value.attr.har_id, 1);
            write_bits(&writer, ops[i]->value.attr.attribute, 8);
        }
    }

    return true;
}

static const char *attr_name[] = {"X Position",        "Y Position",   "X Velocity",  "Y Velocity",
                                  "State ID",          "Animation ID", "Health",      "Stamina",
                                  "Opponent Distance", "Direction",    "ATTR_INVALID"};
rec_har_attr rec_assertion_get_har_attr(const char *key) {
    if(strcmp(key, "xpos") == 0)
        return ATTR_X_POS;
    if(strcmp(key, "ypos") == 0)
        return ATTR_Y_POS;
    if(strcmp(key, "xvel") == 0)
        return ATTR_X_VEL;
    if(strcmp(key, "yvel") == 0)
        return ATTR_Y_VEL;
    if(strcmp(key, "state") == 0)
        return ATTR_STATE_ID;
    if(strcmp(key, "anim") == 0)
        return ATTR_ANIMATION_ID;
    if(strcmp(key, "health") == 0)
        return ATTR_HEALTH;
    if(strcmp(key, "stamina") == 0)
        return ATTR_STAMINA;
    if(strcmp(key, "opp_dist") == 0)
        return ATTR_OPPONENT_DISTANCE;
    if(strcmp(key, "dir") == 0)
        return ATTR_DIRECTION;

    return ATTR_INVALID;
}

int rec_assertion_get_operand(rec_assertion_operand *op, const char *operand, const char *value) {
    if(strcmp(operand, "har1") == 0) {
        op->is_literal = false;
        op->value.attr.har_id = 0;
        op->value.attr.attribute = rec_assertion_get_har_attr(value);
        if(op->value.attr.attribute == ATTR_INVALID) {
            return 2;
        }
        return 0;
    } else if(strcmp(operand, "har2") == 0) {
        op->is_literal = false;
        op->value.attr.har_id = 1;
        op->value.attr.attribute = rec_assertion_get_har_attr(value);
        if(op->value.attr.attribute == ATTR_INVALID) {
            return 2;
        }
        return 0;
    } else if(strcmp(operand, "literal") == 0) {
        op->is_literal = true;
        op->value.literal = atoi(value);
        return 0;
    }

    return 1;
}

// Helper function to print assertions
void rec_assertion_to_str(str *s, const rec_assertion *assertion) {
    const char *op_str;
    switch(assertion->op) {
        case OP_LT:
            op_str = "<";
            break;
        case OP_GT:
            op_str = ">";
            break;
        case OP_EQ:
            op_str = "==";
            break;
        case OP_SET:
            op_str = ":=";
            break;
        default:
            op_str = "??";
            break;
    }

    str_from_c(s, "Assertion: ");

    // Print operand 1
    if(assertion->operand1.is_literal) {
        str_append_format(s, "%d", assertion->operand1.value.literal);
    } else {
        str_append_format(s, "HAR %d's %s", assertion->operand1.value.attr.har_id,
                          attr_name[assertion->operand1.value.attr.attribute]);
    }

    str_append_format(s, " %s ", op_str);

    // Print operand 2
    if(assertion->operand2.is_literal) {
        str_append_format(s, "%d", assertion->operand2.value.literal);
    } else {
        str_append_format(s, "HAR %d's %s", assertion->operand2.value.attr.har_id,
                          attr_name[assertion->operand2.value.attr.attribute]);
    }
}

void print_assertion(const rec_assertion *assertion) {
    str s;
    rec_assertion_to_str(&s, assertion);
    printf("%s\n", str_c(&s));
    str_free(&s);
}

void log_assertion(const rec_assertion *assertion) {
    str s;
    rec_assertion_to_str(&s, assertion);
    log_debug("%s", str_c(&s));
    str_free(&s);
}
