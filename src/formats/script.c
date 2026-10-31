#include "formats/script.h"
#include "formats/error.h"
#include "formats/tag_list.h"
#include "formats/tag_list_helpers.h"
#include "utils/allocator.h"
#include "utils/sstream.h"
#include "utils/str.h"
#include <assert.h>
#include <string.h>

#define SCRIPT_MAX_TAG_LEN 3

static bool tag_str_to_enum(const char *tag, script_tag *out) {
    return script_tag_lookup(tag, (int)strlen(tag), out);
}

void script_create(script *script) {
    assert(script != NULL);
    memset(script, 0, sizeof(*script));
    vector_create(&script->frames, sizeof(script_frame));
}

void script_frame_create(script_frame *frame, const int tick_len, const int sprite) {
    vector_create(&frame->tags, sizeof(script_frame_tag));
    frame->tick_len = tick_len;
    frame->sprite = sprite;
}

int script_frame_clone(const script_frame *src, script_frame *dst) {
    iterator it;
    script_frame_tag *tag;
    vector_iter_begin(&src->tags, &it);
    foreach(it, tag) {
        vector_append(&dst->tags, tag);
    }
    return SD_SUCCESS;
}

int script_clone(const script *src, script *dst) {
    script_create(dst);
    iterator it;
    script_frame *frame;
    vector_iter_begin(&src->frames, &it);
    foreach(it, frame) {
        script_frame new_frame;
        script_frame_create(&new_frame, frame->tick_len, frame->sprite);
        script_frame_clone(frame, &new_frame);
        vector_append(&dst->frames, &new_frame);
    }
    return SD_SUCCESS;
}

void script_frame_free(script_frame *frame) {
    if(frame == NULL) {
        return;
    }
    vector_free(&frame->tags);
}

static void script_frame_tag_create(script_frame_tag *tag) {
    memset(tag, 0, sizeof(script_frame_tag));
}

bool script_frame_add_tag(script_frame *frame, const char *key, int value) {
    script_tag id;
    if(!tag_str_to_enum(key, &id)) {
        return false;
    }
    script_frame_tag tag;
    script_frame_tag_create(&tag);
    tag.key = (uint8_t)id;
    tag.has_param = (uint8_t)tag_descriptor_list[id].has_param;
    if(tag.has_param) {
        tag.value = (int16_t)value;
    }
    vector_append(&frame->tags, &tag);
    return true;
}

void script_free(script *script) {
    if(script == NULL) {
        return;
    }
    iterator it;
    script_frame *frame;
    vector_iter_begin(&script->frames, &it);
    foreach(it, frame) {
        script_frame_free(frame);
    }
    vector_free(&script->frames);
}

int script_append_frame(script *script, int tick_len, int sprite_id) {
    assert(script != NULL);

    script_frame frame;
    script_frame_create(&frame, tick_len, sprite_id);
    vector_append(&script->frames, &frame);
    return SD_SUCCESS;
}

int script_clear_tags(script *script, int frame_id) {
    assert(script != NULL);
    if(frame_id < 0) {
        return SD_INVALID_INPUT;
    }

    script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return SD_INVALID_INPUT;
    }

    vector_clear(&frame->tags);
    return SD_SUCCESS;
}

int script_set_tick_len_at_frame(script *script, int frame_id, int duration) {
    assert(script != NULL);
    if(frame_id < 0) {
        return SD_INVALID_INPUT;
    }

    script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return SD_INVALID_INPUT;
    }

    frame->tick_len = duration;
    return SD_SUCCESS;
}

int script_set_sprite_at_frame(script *script, int frame_id, int sprite_id) {
    assert(script != NULL);
    if(frame_id < 0 || sprite_id < 0 || sprite_id > 25) {
        return SD_INVALID_INPUT;
    }

    script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return SD_INVALID_INPUT;
    }

    frame->sprite = sprite_id;
    return SD_SUCCESS;
}

unsigned script_get_total_ticks(const script *script) {
    return script_get_tick_pos_at_frame(script, vector_size(&script->frames));
}

int script_get_tick_pos_at_frame(const script *script, int frame_id) {
    if(script == NULL) {
        return 0;
    }
    int len = 0;
    script_frame *frame;
    for(int i = 0; i < frame_id; i++) {
        if((frame = vector_get(&script->frames, i)) != NULL) {
            len += frame->tick_len;
        }
    }
    return len;
}

int script_get_tick_len_at_frame(const script *script, int frame_id) {
    if(script == NULL) {
        return 0;
    }

    const script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return 0;
    }

    return frame->tick_len;
}

int script_get_sprite_at_frame(const script *script, int frame_id) {
    if(script == NULL) {
        return 0;
    }

    script_frame *frame = vector_get(&script->frames, frame_id);
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

static bool is_value_start(const char c) {
    return is_numeric(c) || c == '-' || c == '+';
}

static bool parse_tag(script_frame_tag *new, sstream *s) {
    if(!is_tag_letter(sstream_peek(s))) {
        return false;
    }

    // Try the longest matching tag first, since tags can be 1 to 3 characters long.
    int max_len = sstream_left(s);
    if(max_len > SCRIPT_MAX_TAG_LEN) {
        max_len = SCRIPT_MAX_TAG_LEN;
    }
    for(int len = max_len; len > 0; len--) {
        script_tag id;
        if(!script_tag_lookup(sstream_ptr(s), len, &id)) {
            continue;
        }

        // If the tag accepts no params but there are params, just keep searching.
        const int has_param = tag_descriptor_list[id].has_param;
        if(!has_param && is_value_start(sstream_peek_at(s, len))) {
            continue;
        }

        // Tag found. Jump forward, read the value if required, and we're done.
        new->key = (uint8_t)id;
        new->has_param = (uint8_t)has_param;
        sstream_skip(s, len);
        if(has_param) {
            long value;
            sstream_read_long(s, &value, INT16_MIN, INT16_MAX);
            new->value = (int16_t)value;
        }
        return true;
    }
    return false;
}

static bool parse_invalid_tag(script_frame_tag *new, sstream *s) {
    const char ch = sstream_peek(s);
    if(script_invalid_tag_name(ch) == NULL) {
        return false;
    }
    new->key = (uint8_t)TAG_INVALID;
    new->value = (int16_t)ch;
    sstream_skip(s, 1);
    return true;
}

static bool parse_frame(script_frame *frame, sstream *s) {
    const char frame_id = sstream_peek(s);
    if(is_frame_id(frame_id)) {
        sstream_skip(s, 1); // Hop over the frame ID
        frame->sprite = script_letter_to_frame(frame_id);
        long tick_len;
        sstream_read_long(s, &tick_len, INT16_MIN, INT16_MAX);
        frame->tick_len = (int)tick_len;
        sstream_skip(s, 1); // Bypass separator '-'
        return true;
    }
    return false;
}

static void on_fail_seek_end(sstream *s) {
    while(!sstream_eof(s) && sstream_peek(s) != '-') {
        sstream_skip(s, 1);
    }
    sstream_skip(s, 1); // Bypass the '-' (or the end of the buffer)
}

static bool decode_next_frame(script_frame *frame, sstream *s) {
    script_frame_tag tag;
    script_frame_tag_create(&tag);
    while(!sstream_eof(s)) {
        if(parse_frame(frame, s)) {
            return true;
        }
        if(parse_tag(&tag, s)) {
            vector_append(&frame->tags, &tag);
            script_frame_tag_create(&tag);
            continue;
        }
        // There are some invalid tags -- Just read them, so that we can round-trip properly.
        if(parse_invalid_tag(&tag, s)) {
            vector_append(&frame->tags, &tag);
            script_frame_tag_create(&tag);
            continue;
        }
        on_fail_seek_end(s);
        return false;
    }
    return false;
}

static int decode(script *script, sstream *s, int *invalid_pos) {
    int prev = sstream_pos(s);
    while(!sstream_eof(s)) {
        script_frame frame;
        script_frame_create(&frame, 0, 0);
        if(decode_next_frame(&frame, s)) {
            vector_compact(&frame.tags);
            vector_append(&script->frames, &frame);
        } else {
            script_frame_free(&frame);
        }
        if(prev == sstream_pos(s)) {
            // If we are not moving, then something crashed badly.
            if(invalid_pos != NULL) {
                *invalid_pos = sstream_pos(s);
            }
            return SD_ANIM_INVALID_STRING;
        }
        prev = sstream_pos(s);
    }
    vector_compact(&script->frames);

    return SD_SUCCESS;
}

int script_decode_str(script *script, const str *src, int *invalid_pos) {
    assert(script != NULL);
    assert(src != NULL);
    sstream s;
    sstream_open(&s, str_c(src), (int)str_size(src));
    return decode(script, &s, invalid_pos);
}

int script_decode(script *script, const char *input, int *invalid_pos) {
    assert(input != NULL);
    sstream s;
    sstream_open_c(&s, input);
    return decode(script, &s, invalid_pos);
}

int script_encode(const script *script, str *output) {
    assert(script != NULL);
    assert(output != NULL);

    // If there are no frames, then we just stop.
    if(vector_size(&script->frames) <= 0) {
        return SD_SUCCESS;
    }

    // Frames exist. Walk through each, and output tags and ending frame tag + duration
    iterator it;
    script_frame *frame;
    vector_iter_begin(&script->frames, &it);
    foreach(it, frame) {
        script_encode_frame(frame, output);
        str_append_char(output, '-');
    }

    str_cut(output, 1); // Remove the last '-'
    return SD_SUCCESS;
}

int script_encode_frame(const script_frame *frame, str *dst) {
    assert(frame != NULL);
    assert(dst != NULL);

    str tmp;
    iterator tag_it;
    script_frame_tag *tag;
    vector_iter_begin(&frame->tags, &tag_it);
    foreach(tag_it, tag) {
        str_append_c(dst, script_get_frame_tag_name(tag));
        if(tag->has_param) {
            str_from_format(&tmp, "%d", tag->value);
            str_append(dst, &tmp);
            str_free(&tmp);
        }
    }
    str_from_format(&tmp, "%c%d", script_frame_to_letter(frame->sprite), frame->tick_len);
    str_append(dst, &tmp);
    str_free(&tmp);
    return SD_SUCCESS;
}

const script_frame *script_get_frame_at(const script *script, int ticks) {
    if(script == NULL || ticks < 0) {
        return NULL;
    }

    iterator it;
    script_frame *frame;
    int pos = 0;

    vector_iter_begin(&script->frames, &it);
    foreach(it, frame) {
        const int next = pos + frame->tick_len;
        if(pos <= ticks && ticks < next) {
            return frame;
        }
        pos = next;
    }
    return NULL;
}

const script_frame *script_get_frame(const script *script, int frame_number) {
    if(script == NULL || frame_number < 0) {
        return NULL;
    }
    return vector_get(&script->frames, frame_number);
}

int script_get_frame_count(const script *script) {
    if(script == NULL) {
        return 0;
    }
    return (int)vector_size(&script->frames);
}

int script_frame_changed(const script *script, int tick_start, int tick_stop) {
    if(script == NULL || tick_start == tick_stop) {
        return 0;
    }
    const script_frame *frame_a = script_get_frame_at(script, tick_start);
    const script_frame *frame_b = script_get_frame_at(script, tick_stop);
    return frame_a != frame_b;
}

int script_get_frame_index(const script *script, const script_frame *frame) {
    if(script == NULL || frame == NULL) {
        return -1;
    }
    for(unsigned i = 0; i < vector_size(&script->frames); i++) {
        if(vector_get(&script->frames, i) == frame) {
            return i;
        }
    }
    return -1;
}

int script_get_frame_index_at(const script *script, unsigned ticks) {
    if(script == NULL) {
        return -1;
    }

    unsigned pos = 0;
    for(unsigned i = 0; i < vector_size(&script->frames); i++) {
        const script_frame *now = vector_get(&script->frames, i);
        const unsigned next = pos + now->tick_len;
        if(pos <= ticks && ticks < next) {
            return (int)i;
        }
        pos = next;
    }
    return -1;
}

int script_is_last_frame(const script *script, const script_frame *frame) {
    if(script == NULL) {
        return 0;
    }
    return frame == vector_get(&script->frames, vector_size(&script->frames) - 1);
}

int script_is_last_frame_at(const script *script, int ticks) {
    const script_frame *frame = script_get_frame_at(script, ticks);
    return script_is_last_frame(script, frame);
}

int script_is_first_frame(const script *script, const script_frame *frame) {
    if(script == NULL) {
        return 0;
    }
    return script_get_frame_index(script, frame) == 0;
}

int script_is_first_frame_at(const script *script, int ticks) {
    const script_frame *frame = script_get_frame_at(script, ticks);
    return script_is_first_frame(script, frame);
}

const script_frame_tag *script_get_tag_by_id(const script_frame *frame, script_tag id) {
    if(frame == NULL) {
        return NULL;
    }
    iterator it;
    script_frame_tag *now;
    vector_iter_end(&frame->tags, &it);
    foreach_reverse(it, now) {
        if(now->key == (uint8_t)id) {
            return now;
        }
    }
    return NULL;
}

const script_frame_tag *script_get_tag_by_name(const script_frame *frame, const char *tag) {
    if(tag == NULL) {
        return NULL;
    }
    script_tag wanted;
    if(!tag_str_to_enum(tag, &wanted)) {
        return NULL;
    }
    return script_get_tag_by_id(frame, wanted);
}

int script_is_tag_set_by_id(const script_frame *frame, script_tag id) {
    return script_get_tag_by_id(frame, id) != NULL;
}

int script_get_tag_value_by_id(const script_frame *frame, script_tag id) {
    const script_frame_tag *stag = script_get_tag_by_id(frame, id);
    if(stag == NULL) {
        return 0;
    }
    return stag->value;
}

int script_is_tag_set_by_name(const script_frame *frame, const char *tag) {
    return script_get_tag_by_name(frame, tag) != NULL;
}

int script_get_tag_value_by_name(const script_frame *frame, const char *tag) {
    const script_frame_tag *stag = script_get_tag_by_name(frame, tag);
    if(stag == NULL) {
        return 0;
    }
    return stag->value;
}

int script_get_next_frame_with_sprite(const script *script, int sprite_id, unsigned current_tick) {
    if(script == NULL) {
        return -1;
    }
    if(sprite_id < 0) {
        return -1;
    }
    if(current_tick > script_get_total_ticks(script)) {
        return -1;
    }

    unsigned pos = 0;
    for(unsigned i = 0; i < vector_size(&script->frames); i++) {
        const script_frame *frame = vector_get(&script->frames, i);
        const unsigned next = pos + frame->tick_len;
        if(current_tick < pos && sprite_id == frame->sprite) {
            return (int)i;
        }
        pos = next;
    }

    return -1;
}

int script_get_next_frame_with_tag_id(const script *script, script_tag id, uint32_t current_tick) {
    if(script == NULL) {
        return -1;
    }
    if(current_tick > script_get_total_ticks(script)) {
        return -1;
    }

    unsigned pos = 0;
    for(unsigned i = 0; i < vector_size(&script->frames); i++) {
        const script_frame *frame = vector_get(&script->frames, i);
        const unsigned next = pos + frame->tick_len;
        if(current_tick < pos && script_is_tag_set_by_id(frame, id)) {
            return (int)i;
        }
        pos = next;
    }

    return -1;
}

int script_get_next_frame_with_tag(const script *script, const char *tag, uint32_t current_tick) {
    if(tag == NULL) {
        return -1;
    }
    script_tag id;
    if(!tag_str_to_enum(tag, &id)) {
        return -1;
    }
    return script_get_next_frame_with_tag_id(script, id, current_tick);
}

int script_delete_tag(script *script, int frame_id, const char *tag) {
    assert(script != NULL);
    assert(tag != NULL);
    if(frame_id < 0) {
        return SD_INVALID_INPUT;
    }

    script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return SD_INVALID_INPUT;
    }

    script_tag wanted;
    if(!tag_str_to_enum(tag, &wanted)) {
        return SD_SUCCESS; // Nothing to delete for an unknown tag.
    }

    iterator it;
    script_frame_tag *now;
    vector_iter_begin(&frame->tags, &it);
    foreach(it, now) {
        if(now->key == (uint8_t)wanted) {
            vector_delete(&frame->tags, &it);
            return SD_SUCCESS;
        }
    }
    return SD_SUCCESS;
}

int script_set_tag(script *script, int frame_id, const char *tag, int value) {
    assert(script != NULL);
    assert(tag != NULL);
    if(frame_id < 0) {
        return SD_INVALID_INPUT;
    }

    // Get frame
    script_frame *frame = vector_get(&script->frames, frame_id);
    if(frame == NULL) {
        return SD_INVALID_INPUT;
    }

    // Get tag information
    script_tag id;
    if(!tag_str_to_enum(tag, &id)) {
        return SD_INVALID_INPUT;
    }
    script_frame_tag new;
    script_frame_tag_create(&new);
    new.key = (uint8_t)id;
    new.has_param = (uint8_t)tag_descriptor_list[id].has_param;
    if(new.has_param) {
        new.value = (int16_t)value;
    }

    // Delete old tag (if exists), then add new.
    script_delete_tag(script, frame_id, tag);
    vector_append(&frame->tags, &new);
    return SD_SUCCESS;
}

int script_letter_to_frame(char letter) {
    return (int)(letter - 'A');
}

char script_frame_to_letter(int frame_id) {
    assert(frame_id >= 0 && frame_id <= 25);
    return (char)(frame_id + 'A');
}
