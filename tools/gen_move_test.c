/**
 * @file gen_move_test.c
 * @brief Generate deterministic REC files for controller and replay tests.
 *
 * Usage: gen_move_test <har_id> <opponent_id> <output_file> [options] [hex_byte ...]
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats/error.h"
#include "formats/rec.h"
#include "formats/rec_assertion.h"
#include "utils/path.h"
#include "utils/vector.h"

#define MAX_INPUTS 256
#define MAX_ASSERTIONS 256

static int insert_lookup10(sd_rec_file *rec, uint32_t tick, const uint8_t *data) {
    sd_rec_move move;
    memset(&move, 0, sizeof(move));
    move.player_id = 0;
    move.tick = tick;

    char *extra_data = sd_rec_set_lookup_id(&move, 10);
    if(extra_data == NULL) {
        return SD_INVALID_INPUT;
    }
    memcpy(extra_data, data, sd_rec_extra_len(10));
    return sd_rec_insert_action_at_tick(rec, &move);
}

static int insert_seed(sd_rec_file *rec, uint32_t seed) {
    uint8_t data[8] = {0};
    data[0] = REC_LOOKUP10_SETRANDOM_BYTE;
    memcpy(data + 4, &seed, sizeof(seed));
    return insert_lookup10(rec, 0, data);
}

static int parse_operand(const char *text, rec_assertion_operand *out) {
    if(strncmp(text, "har1.", 5) == 0 || strncmp(text, "har2.", 5) == 0) {
        out->is_literal = false;
        out->value.attr.har_id = text[3] - '1';
        out->value.attr.attribute = rec_assertion_get_har_attr(text + 5);
        return out->value.attr.attribute == ATTR_INVALID ? SD_INVALID_INPUT : SD_SUCCESS;
    }

    out->is_literal = true;
    out->value.literal = (int16_t)atoi(text);
    return SD_SUCCESS;
}

static int insert_assertion(sd_rec_file *rec, const char *spec) {
    const char *at = strrchr(spec, '@');
    if(at == NULL) {
        return SD_INVALID_INPUT;
    }

    uint32_t tick = (uint32_t)strtoul(at + 1, NULL, 10);
    char expr[256];
    size_t expr_len = (size_t)(at - spec);
    if(expr_len == 0 || expr_len >= sizeof(expr)) {
        return SD_INVALID_INPUT;
    }
    memcpy(expr, spec, expr_len);
    expr[expr_len] = '\0';

    const char *op_pos;
    rec_assertion_operator op;
    size_t op_len;
    if((op_pos = strstr(expr, "==")) != NULL) {
        op = OP_EQ;
        op_len = 2;
    } else if((op_pos = strstr(expr, ":=")) != NULL) {
        op = OP_SET;
        op_len = 2;
    } else if((op_pos = strchr(expr, '<')) != NULL) {
        op = OP_LT;
        op_len = 1;
    } else if((op_pos = strchr(expr, '>')) != NULL) {
        op = OP_GT;
        op_len = 1;
    } else {
        return SD_INVALID_INPUT;
    }

    char lhs[128];
    char rhs[128];
    size_t lhs_len = (size_t)(op_pos - expr);
    if(lhs_len == 0 || lhs_len >= sizeof(lhs)) {
        return SD_INVALID_INPUT;
    }
    memcpy(lhs, expr, lhs_len);
    lhs[lhs_len] = '\0';
    snprintf(rhs, sizeof(rhs), "%s", op_pos + op_len);

    rec_assertion assertion;
    memset(&assertion, 0, sizeof(assertion));
    assertion.op = op;
    if(parse_operand(lhs, &assertion.operand1) != SD_SUCCESS ||
       parse_operand(rhs, &assertion.operand2) != SD_SUCCESS) {
        return SD_INVALID_INPUT;
    }

    uint8_t data[8] = {0};
    if(!encode_assertion(&assertion, data)) {
        return SD_INVALID_INPUT;
    }
    return insert_lookup10(rec, tick, data);
}

static unsigned max_event_tick(const sd_rec_file *rec) {
    unsigned max_tick = 0;
    for(unsigned i = 0; i < vector_size(&rec->moves); i++) {
        const sd_rec_move *move = vector_get(&rec->moves, i);
        if(move->tick > max_tick) {
            max_tick = move->tick;
        }
    }
    return max_tick;
}

int main(int argc, char *argv[]) {
    if(argc < 4) {
        fprintf(stderr, "Usage: %s <har_id> <opponent_id> <output_file> [options] [hex_byte ...]\n", argv[0]);
        fprintf(stderr, "Options: --seed <n> --ticks <n> --assert <expr@tick>\n");
        return EXIT_FAILURE;
    }

    int har_id = atoi(argv[1]);
    int opponent_id = atoi(argv[2]);
    const char *output_file = argv[3];
    if(har_id < 0 || har_id >= 11 || opponent_id < 0 || opponent_id >= 11) {
        fprintf(stderr, "Invalid HAR IDs\n");
        return EXIT_FAILURE;
    }

    int inputs[MAX_INPUTS];
    int input_count = 0;
    const char *assertions[MAX_ASSERTIONS];
    int assertion_count = 0;
    uint32_t seed = 0;
    uint32_t explicit_ticks = 0;
    bool has_seed = false;

    for(int i = 4; i < argc; i++) {
        if(strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = (uint32_t)strtoul(argv[++i], NULL, 10);
            has_seed = true;
        } else if(strcmp(argv[i], "--ticks") == 0 && i + 1 < argc) {
            explicit_ticks = (uint32_t)strtoul(argv[++i], NULL, 10);
        } else if(strcmp(argv[i], "--assert") == 0 && i + 1 < argc) {
            if(assertion_count >= MAX_ASSERTIONS) {
                fprintf(stderr, "Too many assertions\n");
                return EXIT_FAILURE;
            }
            assertions[assertion_count++] = argv[++i];
        } else if(argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return EXIT_FAILURE;
        } else if(input_count < MAX_INPUTS) {
            inputs[input_count++] = (int)strtol(argv[i], NULL, 16);
        } else {
            fprintf(stderr, "Too many input bytes\n");
            return EXIT_FAILURE;
        }
    }

    if(input_count == 0 && assertion_count == 0 && !has_seed) {
        fprintf(stderr, "No inputs, assertions, or seed specified\n");
        return EXIT_FAILURE;
    }

    sd_rec_file rec;
    sd_rec_create(&rec);
    rec.game_mode = REC_GAMEMODE_ARCADE;
    rec.arena_id = 0;
    rec.p1_controller = REC_CONTROLLER_CUSTOM_KEYBOARD1;
    rec.p2_controller = REC_CONTROLLER_AI;
    rec.p2_controller_ = REC_CONTROLLER_AI;
    rec.throw_range = 100;
    rec.hit_pause = 8;
    rec.block_damage = 100;
    rec.vitality = 100;
    rec.jump_height = 100;
    rec.knock_down = 3;
    rec.rehit_mode = 1;
    rec.def_throws = 1;
    rec.power[0] = 5;
    rec.power[1] = 5;
    rec.hazards = 0;
    rec.round_type = 0;
    rec.hyper_mode = 0;
    rec.pilots[0].info.har_id = har_id;
    rec.pilots[0].info.power = 15;
    rec.pilots[0].info.agility = 15;
    rec.pilots[0].info.endurance = 15;
    rec.pilots[1].info.har_id = opponent_id;
    rec.pilots[1].info.power = 15;
    rec.pilots[1].info.agility = 15;
    rec.pilots[1].info.endurance = 15;

    if(has_seed && insert_seed(&rec, seed) != SD_SUCCESS) {
        fprintf(stderr, "Failed to insert seed record\n");
        sd_rec_free(&rec);
        return EXIT_FAILURE;
    }

    for(int i = 0; i < input_count; i++) {
        sd_rec_move move;
        memset(&move, 0, sizeof(move));
        move.player_id = 0;
        move.tick = 50 + (unsigned)i;
        char *extra_data = sd_rec_set_lookup_id(&move, 2);
        if(extra_data == NULL) {
            sd_rec_free(&rec);
            return EXIT_FAILURE;
        }
        extra_data[0] = (uint8_t)inputs[i];
        if(sd_rec_insert_action_at_tick(&rec, &move) != SD_SUCCESS) {
            fprintf(stderr, "Failed to insert action\n");
            sd_rec_free(&rec);
            return EXIT_FAILURE;
        }
    }

    for(int i = 0; i < assertion_count; i++) {
        if(insert_assertion(&rec, assertions[i]) != SD_SUCCESS) {
            fprintf(stderr, "Invalid assertion spec: %s\n", assertions[i]);
            sd_rec_free(&rec);
            return EXIT_FAILURE;
        }
    }

    unsigned total_ticks = explicit_ticks != 0 ? explicit_ticks : max_event_tick(&rec) + 150;
    sd_rec_finish(&rec, total_ticks);

    path output_path;
    path_from_c(&output_path, output_file);
    int result = sd_rec_save(&rec, &output_path);
    sd_rec_free(&rec);
    if(result != SD_SUCCESS) {
        fprintf(stderr, "Failed to save REC file to %s\n", output_file);
        return EXIT_FAILURE;
    }

    printf("Generated REC: %s (ticks: %u, inputs: %d, assertions: %d)\n",
           output_file, total_ticks, input_count, assertion_count);
    return EXIT_SUCCESS;
}
