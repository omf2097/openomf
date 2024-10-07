#include "formats/script.h"
#include "formats/error.h"
#include "formats/taglist.h"
#include "utils/allocator.h"
#include "utils/str.h"
#include <ctype.h>
#include <string.h>

#define INVALID_TAG_COUNT 5
static const char *INVALID_TAGS[INVALID_TAG_COUNT] = {"u", "c", "p", "o", "z"};

int sd_script_create(sd_script *script) {
    if(script == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(script, 0, sizeof(sd_script));
    vector_create(&script->frames, sizeof(sd_script_frame));
    return SD_SUCCESS;
}

static void sd_script_frame_create(sd_script_frame *frame) {
    vector_create(&frame->tags, sizeof(sd_script_tag));
    frame->tick_len = 0;
    frame->sprite = 0;
}

int sd_script_frame_clone(sd_script_frame *src, sd_script_frame *dst) {
    iterator it;
    sd_script_tag *tag;
    vector_iter_begin(&src->tags, &it);
    while((tag = iter_next(&it)) != NULL) {
        vector_append(&dst->tags, tag);
    }
    return SD_SUCCESS;
}

int sd_script_clone(sd_script *src, sd_script *dst) {
    sd_script_create(dst);
    iterator it;
    sd_script_frame *frame;
    vector_iter_begin(&src->frames, &it);
    while((frame = iter_next(&it)) != NULL) {
        sd_script_frame new_frame;
        sd_script_frame_create(&new_frame);
        new_frame.sprite = frame->sprite;
        new_frame.tick_len = frame->tick_len;
        sd_script_frame_clone(frame, &new_frame);
        vector_append(&dst->frames, &new_frame);
    }
    return SD_SUCCESS;
}

static void sd_script_frame_free(sd_script_frame *frame) {
    if(frame == NULL)
        return;
    vector_free(&frame->tags);
}

static void sd_script_tag_create(sd_script_tag *tag) {
    memset(tag, 0, sizeof(sd_script_tag));
}

void sd_script_free(sd_script *script) {
    if(script == NULL)
        return;
    iterator it;
    sd_script_frame *frame;
    vector_iter_begin(&script->frames, &it);
    while((frame = iter_next(&it)) != NULL) {
        sd_script_frame_free(frame);
    }
    vector_free(&script->frames);
}

int sd_script_append_frame(sd_script *script, int tick_len, int sprite_id) {
    if(script == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_script_frame frame;
    sd_script_frame_create(&frame);
    frame.tick_len = tick_len;
    frame.sprite = sprite_id;
    vector_append(&script->frames, &frame);
    return SD_SUCCESS;
}

int sd_script_clear_tags(sd_script *script, int frame_id) {
    if(script == NULL || frame_id < 0) {
        return SD_INVALID_INPUT;
    }

    sd_script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return SD_INVALID_INPUT;
    }

    vector_clear(&frame->tags);
    return SD_SUCCESS;
}

int sd_script_set_tick_len_at_frame(sd_script *script, int frame_id, int duration) {
    if(script == NULL || frame_id < 0) {
        return SD_INVALID_INPUT;
    }

    sd_script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return SD_INVALID_INPUT;
    }

    frame->tick_len = duration;
    return SD_SUCCESS;
}

int sd_script_set_sprite_at_frame(sd_script *script, int frame_id, int sprite_id) {
    if(script == NULL || frame_id < 0 || sprite_id < 0 || sprite_id > 25) {
        return SD_INVALID_INPUT;
    }

    sd_script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return SD_INVALID_INPUT;
    }

    frame->sprite = sprite_id;
    return SD_SUCCESS;
}

int sd_script_get_total_ticks(const sd_script *script) {
    return sd_script_get_tick_pos_at_frame(script, vector_size(&script->frames));
}

int sd_script_get_tick_pos_at_frame(const sd_script *script, int frame_id) {
    if(script == NULL) {
        return 0;
    }
    int len = 0;
    sd_script_frame *frame;
    for(int i = 0; i < frame_id; i++) {
        if((frame = vector_get(&script->frames, i)) != NULL) {
            len += frame->tick_len;
        }
    }
    return len;
}

int sd_script_get_tick_len_at_frame(const sd_script *script, int frame_id) {
    if(script == NULL) {
        return 0;
    }

    sd_script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return 0;
    }

    return frame->tick_len;
}

int sd_script_get_sprite_at_frame(const sd_script *script, int frame_id) {
    if(script == NULL) {
        return 0;
    }

    sd_script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return 0;
    }

    return frame->sprite;
}

static bool is_frame_id(const char c) {
    return c >= 'A' && c <= 'Z';
}

static bool is_tag_letter(const char c) {
    return c >= 'a' && c <= 'z';
}

static bool is_numeric(const char c) {
    return c >= '0' && c <= '9';
}

static int find_numeric_span(const str *src, int start) {
    int end = start;
    char ch = str_at(src, start);
    if(ch == '-' || ch == '+') {
        end++;
    }
    do {
        ch = str_at(src, end++);
    } while(is_numeric(ch));
    return end - 1;
}

static int read_int(const str *src, int *original) {
    int value = 0;
    int start = *original;
    int end = find_numeric_span(src, start);
    if(start == end) {
        return 0; // If we found no number, just return default 0.
    }

    if(!str_to_int(src, start, &value)) {
        value = 0; // This should never really happen.
    }
    *original = end;
    return value;
}

static bool test_tag_slice(const char *test, int size, sd_script_tag *new, str *src, int *now) {
    if(sd_tag_info(test + *now, size, &new->has_param, &new->key, &new->desc) == 0) {
        const int jmp = *now + strlen(new->key);
        // Ensure that tag has no value, if value is not desired.
        if(!new->has_param && find_numeric_span(src, jmp) > jmp) {
            return false;
        }

        // Okay, tag found. Jump forward, read the value (if required), and we're done.
        *now = jmp;
        if(new->has_param) {
            new->value = read_int(src, now);
        }
        return true;
    }
    return false;
}

static bool parse_tag(sd_script_tag *new, str *src, int *now) {
    if(!is_tag_letter(str_at(src, *now))) {
        return false;
    }

    // Check if tag is legit.
    for(int m = 3; m > 0; m--) {
        // str_from_slice(&test, src, *now, *now + m);
        if(strncmp(src->data + *now, "usw", m) == 0 && find_numeric_span(src, *now + m) > *now + m) {
            // Fixup rare usw30/usw case, which can be u + sw30 or us + w
            // str_free(&test);
            return false;
        }
        if(test_tag_slice(src->data, m, new, src, now)) {
            return true;
        }
    }
    return false;
}

static bool parse_invalid_tag(sd_script_tag *new, str *src, int *now) {
    if(!is_tag_letter(str_at(src, *now))) {
        return false;
    }

    // Check if tag is an invalid tag.
    str test;
    str_from_slice(&test, src, *now, *now + 1);
    for(int i = 0; i < INVALID_TAG_COUNT; i++) {
        const char *tag = INVALID_TAGS[i];
        if(str_equal_c(&test, tag)) {
            new->key = tag;
            *now += strlen(tag);
            str_free(&test);
            return true;
        }
    }
    str_free(&test);
    return false;
}

static bool parse_frame(sd_script_frame *frame, str *src, int *now) {
    const char frame_id = str_at(src, *now);
    if(is_frame_id(frame_id)) {
        (*now)++; // Hop over the frame ID
        frame->sprite = sd_script_letter_to_frame(frame_id);
        frame->tick_len = read_int(src, now);
        (*now)++; // Bypass separator '-'
        return true;
    }
    return false;
}

static bool try_parse_bad_frame(sd_script_frame *frame, str *src, int *now) {
    // First, try to snoop without actually moving the pointer.
    const char frame_id = str_at(src, *now);
    if(!is_tag_letter(frame_id)) { // Lowercase instead of uppercase (bug)
        return false;
    }
    const char next_num = str_at(src, (*now) + 1);
    if(!is_numeric(next_num)) { // Next char must be numeric in this special case.
        return false;
    }

    // Okay, we got it. Read the frame.
    (*now)++; // Hop over the frame ID
    frame->sprite = sd_script_letter_to_frame(toupper(frame_id));
    frame->tick_len = read_int(src, now);
    (*now)++; // Bypass separator '-'
    return true;
}

static bool parse_spurious_dash(str *src, int *now) {
    const char ch = str_at(src, *now);
    if(ch == '-') {
        (*now)++;
        return true;
    }
    return false;
}

int sd_script_decode(sd_script *script, const char *input, int *invalid_pos) {
    if(script == NULL || input == NULL)
        return SD_INVALID_INPUT;

    str src;
    sd_script_frame frame;
    sd_script_tag tag;
    str_from_c(&src, input);
    sd_script_frame_create(&frame);
    sd_script_tag_create(&tag);

    int now = 0;
    while(now < str_size(&src)) {
        if(parse_frame(&frame, &src, &now)) {
            vector_append(&script->frames, &frame);
            sd_script_frame_create(&frame);
            continue;
        }
        if(parse_tag(&tag, &src, &now)) {
            vector_append(&frame.tags, &tag);
            sd_script_tag_create(&tag);
            continue;
        }
        // There are some invalid tags -- Just read them, so that we can round-trip properly.
        if(parse_invalid_tag(&tag, &src, &now)) {
            vector_append(&frame.tags, &tag);
            sd_script_tag_create(&tag);
            continue;
        }
        // Some string(s) have spurious dashes -- ignore the dashes in those cases.
        if(parse_spurious_dash(&src, &now)) {
            continue;
        }
        // There are a couple of cases where uppercase frame letter is lowercase. Try to fix.
        if(try_parse_bad_frame(&frame, &src, &now)) {
            vector_append(&script->frames, &frame);
            sd_script_frame_create(&frame);
            continue;
        }
        goto failed_parse;
    }

    str_free(&src);
    sd_script_frame_free(&frame);
    return SD_SUCCESS;

failed_parse:
    if(invalid_pos != NULL) {
        *invalid_pos = now;
    }
    str_free(&src);
    sd_script_frame_free(&frame);
    return SD_ANIM_INVALID_STRING;
}

int sd_script_encode(const sd_script *script, str *output) {
    if(script == NULL || output == NULL)
        return SD_INVALID_INPUT;

    // If there are no frames, then we just stop.
    if(vector_size(&script->frames) <= 0) {
        return SD_SUCCESS;
    }

    // Frames exist. Walk through each, and output tags and ending frame tag + duration
    str tmp;
    iterator frame_it, tag_it;
    sd_script_frame *frame;
    sd_script_tag *tag;

    vector_iter_begin(&script->frames, &frame_it);
    while((frame = iter_next(&frame_it)) != NULL) {
        vector_iter_begin(&frame->tags, &tag_it);
        while((tag = iter_next(&tag_it)) != NULL) {
            str_append_c(output, tag->key);
            if(tag->has_param) {
                str_from_format(&tmp, "%d", tag->value);
                str_append(output, &tmp);
                str_free(&tmp);
            }
        }
        str_from_format(&tmp, "%c%d-", sd_script_frame_to_letter(frame->sprite), frame->tick_len);
        str_append(output, &tmp);
        str_free(&tmp);
    }

    str_cut(output, 1); // Remove the last '-'
    return SD_SUCCESS;
}

const sd_script_frame *sd_script_get_frame_at(const sd_script *script, int ticks) {
    if(script == NULL)
        return NULL;
    if(ticks < 0)
        return NULL;

    iterator it;
    sd_script_frame *frame;
    int next, pos = 0;

    vector_iter_begin(&script->frames, &it);
    while((frame = iter_next(&it)) != NULL) {
        next = pos + frame->tick_len;
        if(pos <= ticks && ticks < next) {
            return frame;
        }
        pos = next;
    }
    return NULL;
}

const sd_script_frame *sd_script_get_frame(const sd_script *script, int frame_number) {
    if(script == NULL || frame_number < 0) {
        return NULL;
    }
    return vector_get(&script->frames, frame_number);
}

int sd_script_frame_changed(const sd_script *script, int tick_start, int tick_stop) {
    if(script == NULL)
        return 0;
    if(tick_start == tick_stop)
        return 0;
    const sd_script_frame *frame_a = sd_script_get_frame_at(script, tick_start);
    const sd_script_frame *frame_b = sd_script_get_frame_at(script, tick_stop);
    return frame_a != frame_b;
}

int sd_script_get_frame_index(const sd_script *script, const sd_script_frame *frame) {
    if(script == NULL || frame == NULL)
        return -1;
    for(int i = 0; i < vector_size(&script->frames); i++) {
        if(vector_get(&script->frames, i) == frame) {
            return i;
        }
    }
    return -1;
}

int sd_script_get_frame_index_at(const sd_script *script, int ticks) {
    if(script == NULL || ticks < 0)
        return -1;

    int next, pos = 0;
    for(int i = 0; i < vector_size(&script->frames); i++) {
        sd_script_frame *now = vector_get(&script->frames, i);
        next = pos + now->tick_len;
        if(pos <= ticks && ticks < next) {
            return i;
        }
        pos = next;
    }
    return -1;
}

int sd_script_is_last_frame(const sd_script *script, const sd_script_frame *frame) {
    if(script == NULL)
        return 0;
    return frame == vector_get(&script->frames, vector_size(&script->frames) - 1);
}

int sd_script_is_last_frame_at(const sd_script *script, int ticks) {
    const sd_script_frame *frame = sd_script_get_frame_at(script, ticks);
    return sd_script_is_last_frame(script, frame);
}

int sd_script_is_first_frame(const sd_script *script, const sd_script_frame *frame) {
    if(script == NULL)
        return 0;
    return sd_script_get_frame_index(script, frame) == 0;
}

int sd_script_is_first_frame_at(const sd_script *script, int ticks) {
    const sd_script_frame *frame = sd_script_get_frame_at(script, ticks);
    return sd_script_is_first_frame(script, frame);
}

const sd_script_tag *sd_script_get_tag(const sd_script_frame *frame, const char *tag) {
    if(frame == NULL || tag == NULL) {
        return NULL;
    }
    iterator it;
    sd_script_tag *now;
    vector_iter_begin(&frame->tags, &it);
    while((now = iter_next(&it)) != NULL) {
        if(strcmp(tag, now->key) == 0) {
            return now;
        }
    }
    return NULL;
}

int sd_script_isset(const sd_script_frame *frame, const char *tag) {
    return sd_script_get_tag(frame, tag) != NULL;
}

int sd_script_get(const sd_script_frame *frame, const char *tag) {
    const sd_script_tag *stag = sd_script_get_tag(frame, tag);
    if(stag == NULL) {
        return 0;
    }
    return stag->value;
}

int sd_script_next_frame_with_sprite(const sd_script *script, int sprite_id, int current_tick) {
    if(script == NULL)
        return -1;
    if(sprite_id < 0)
        return -1;
    if(current_tick > sd_script_get_total_ticks(script))
        return -1;

    int next, pos = 0;
    sd_script_frame *frame;
    for(int i = 0; i < vector_size(&script->frames); i++) {
        frame = vector_get(&script->frames, i);
        next = pos + frame->tick_len;
        if(current_tick < pos && sprite_id == frame->sprite) {
            return i;
        }
        pos = next;
    }

    return -1;
}

int sd_script_next_frame_with_tag(const sd_script *script, const char *tag, int current_tick) {
    if(script == NULL || tag == NULL)
        return -1;
    if(current_tick > sd_script_get_total_ticks(script))
        return -1;

    int next, pos = 0;
    sd_script_frame *frame;
    for(int i = 0; i < vector_size(&script->frames); i++) {
        frame = vector_get(&script->frames, i);
        next = pos + frame->tick_len;
        if(current_tick < pos && sd_script_isset(frame, tag)) {
            return i;
        }
        pos = next;
    }

    return -1;
}

int sd_script_delete_tag(sd_script *script, int frame_id, const char *tag) {
    if(script == NULL || tag == NULL || frame_id < 0)
        return SD_INVALID_INPUT;

    sd_script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return SD_INVALID_INPUT;
    }

    iterator it;
    sd_script_tag *now;
    vector_iter_begin(&frame->tags, &it);
    while((now = iter_next(&it)) != NULL) {
        if(strcmp(now->key, tag) == 0) {
            vector_delete(&frame->tags, &it);
            return SD_SUCCESS;
        }
    }
    return SD_SUCCESS;
}

int sd_script_set_tag(sd_script *script, int frame_id, const char *tag, int value) {
    if(script == NULL || tag == NULL || frame_id < 0)
        return SD_INVALID_INPUT;

    // Get frame
    sd_script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return SD_INVALID_INPUT;
    }

    // Get tag information
    sd_script_tag new;
    if(sd_tag_info(tag, strlen(tag), &new.has_param, &new.key, &new.desc) != SD_SUCCESS) {
        return SD_INVALID_INPUT;
    }
    if(new.has_param) {
        new.value = value;
    }

    // Delete old tag (if exists), then add new.
    sd_script_delete_tag(script, frame_id, tag);
    vector_append(&frame->tags, &new);
    return SD_SUCCESS;
}

int sd_script_letter_to_frame(char letter) {
    return (int)(letter - 'A');
}

char sd_script_frame_to_letter(int frame_id) {
    return (char)(frame_id + 'A');
}
