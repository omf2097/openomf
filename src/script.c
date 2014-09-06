#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shadowdive/script.h"
#include "shadowdive/error.h"
#include "shadowdive/taglist.h"

static int read_next_int(const char *str, int *pos) {
    int opos = 0;
    char buf[20];
    memset(buf, 0, 20);
    if(str[*pos] == '-' && str[(*pos)+1] >= '0' && str[(*pos)+1] <= '9') {
        buf[opos] = str[*pos];
        (*pos)++;
        opos++;
    }
    if(str[*pos] == '+') {
        (*pos)++;
    }
    while(str[*pos] >= '0' && str[*pos] <= '9') {
        buf[opos] = str[*pos];
        (*pos)++;
        opos++;
    }

    if(opos == 0)
        return 0;
    return atoi(buf);
}

int sd_script_create(sd_script *script) {
    if(script == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(script, 0, sizeof(sd_script));
    return SD_SUCCESS;
}

void sd_script_free(sd_script *script) {
    if(script == NULL)
        return;
    for(int i = 0; i < script->frame_count; i++) {
        free(script->frames[i].tags);
    }
    free(script->frames);
}

int sd_script_get_total_ticks(const sd_script *script) {
    if(script == NULL)
        return 0;
    int len = 0;
    for(int i = 0; i < script->frame_count; i++) {
        len += script->frames[i].tick_len;
    }
    return len;
}

static void _create_tag(sd_script_frame *frame, int number) {
    size_t newsize = sizeof(sd_script_tag) * (number + 1);
    frame->tags = realloc(frame->tags, newsize);
    memset(&frame->tags[number], 0, sizeof(sd_script_tag));
}

static void _create_frame(sd_script *script, int number) {
    size_t newsize = sizeof(sd_script_frame) * (number + 1);
    script->frames = realloc(script->frames, newsize);
    memset(&script->frames[number], 0, sizeof(sd_script_frame));
}

int sd_script_decode(sd_script *script, const char* str, int *inv_pos) {
    if(script == NULL || str == NULL)
        return SD_INVALID_INPUT;

    char test[4];
    int len = strlen(str);
    int i = 0;
    int req_param;
    int has_end = 0;
    const char *desc = NULL;
    const char *tag = NULL;

    int frame_number = 0;
    int tag_number = 0;
    _create_frame(script, frame_number);
    while(i < len) {
        if(str[i] >= 'A' && str[i] <= 'Z') {
            // Set the length and id for this frame
            script->frames[frame_number].sprite = str[i++] - 65;
            script->frames[frame_number].tick_len = read_next_int(str, &i);
            tag_number = 0;

            // Create a new frame if necessary
            i++;
            if(i < len) {
                frame_number++;
                _create_frame(script, frame_number);
                has_end = 0;
            } else {
                has_end = 1;
            }
            script->frame_count++;
            continue;
        }
        if(str[i] >= 'a' && str[i] <= 'z') {
            int found = 0;
            for(int k = 3; k > 0; k--) {
                memcpy(test, str+i, k);
                test[k] = 0;

                // See if the current tag matches with anything.
                if(sd_tag_info(test, &req_param, &tag, &desc) == 0) {
                    has_end = 0;
                    i += k;
                    found = 1;

                    // Add entry for tag
                    sd_script_frame *frame = &script->frames[frame_number];
                    _create_tag(frame, tag_number);
                    frame->tag_count++;

                    // Set values
                    frame->tags[tag_number].key = tag;
                    frame->tags[tag_number].desc = desc;
                    frame->tags[tag_number].has_param = req_param;
                    if(req_param) {
                        frame->tags[tag_number].value = read_next_int(str, &i);
                    }
                    tag_number++;
                    k = 0; // Stop here.
                }
            }
            if(!found) {
                // Handle known filler tags
                if(strcmp(test, "u") == 0) { i++; continue; }
                if(strcmp(test, "c") == 0) { i++; continue; }
                if(strcmp(test, "p") == 0) { i++; continue; }
                if(strcmp(test, "o") == 0) { i++; continue; }
                if(strcmp(test, "z") == 0) { i++; continue; }

                // Could do nothing about it.
                if(inv_pos != NULL)
                    *inv_pos = i;
                return SD_INVALID_TAG;
            }
            continue;
        }

        // Should never get here
        i++;
    }
    // Make sure the last frame is complete.
    // If we don't, do some catastrophe management
    if(!has_end) {
        script->frame_count += 1; // Make sure our last entry is freed
        return SD_ANIM_INVALID_STRING;
    }
    return SD_SUCCESS;
}

int sd_script_encode(const sd_script *script, char* str) {
    if(script == NULL || str == NULL)
        return SD_INVALID_INPUT;

    int s = 0;
    for(int i = 0; i < script->frame_count; i++) {
        sd_script_frame *frame = &script->frames[i];
        for(int k = 0; k < frame->tag_count; k++) {
            sd_script_tag *tag = &frame->tags[k];
            s += sprintf(str + s, "%s", tag->key);
            if(tag->has_param) {
                s += sprintf(str + s, "%d", tag->value);
            }
        }
        s += sprintf(str + s, "%c%d-", frame->sprite + 65, frame->tick_len);
    }

    // Overwrite the last '-'
    str[--s] = 0;

    return SD_SUCCESS;
}

int sd_script_encoded_length(const sd_script *script) {
    if(script == NULL)
        return 0;

    int s = 0;
    char tmp[20];
    for(int i = 0; i < script->frame_count; i++) {
        sd_script_frame *frame = &script->frames[i];
        for(int k = 0; k < frame->tag_count; k++) {
            sd_script_tag *tag = &frame->tags[k];
            s += strlen(tag->key); // Tag length
            if(tag->has_param) {
                s += sprintf(tmp, "%d", tag->value); // Tag value length
            }
        }
        s += 2; // sprite key and the '-' char
        s += sprintf(tmp, "%d", frame->tick_len); // Tick length char count
    }
    s--; // Minus the last '-'
    return s;
}

const sd_script_frame* sd_script_get_frame_at(const sd_script *script, int ticks) {
    if(script == NULL)
        return NULL;
    if(ticks < 0)
        return NULL;

    int pos = 0;
    int next = 0;
    for(int i = 0; i < script->frame_count; i++) {
        next = pos + script->frames[i].tick_len;
        if(pos <= ticks && ticks < next) {
            return &script->frames[i];
        }
        pos = next;
    }
    return NULL;
}

const sd_script_frame* sd_script_get_frame(const sd_script *script, int frame_number) {
    if(script == NULL || frame_number < 0 || frame_number >= script->frame_count) {
        return NULL;
    }
    return &script->frames[frame_number];
}

int sd_script_frame_changed(const sd_script *script, int tick_start, int tick_stop) {
    if(script == NULL)
        return 0;
    if(tick_start == tick_stop)
        return 0;
    const sd_script_frame *frame_a = sd_script_get_frame_at(script, tick_start);
    const sd_script_frame *frame_b = sd_script_get_frame_at(script, tick_stop);
    return (frame_a != frame_b);
}

int sd_script_get_frame_index(const sd_script *script, const sd_script_frame *frame) {
    if(script == NULL || frame == NULL)
        return -1;
    for(int i = 0; i < script->frame_count; i++) {
        if(&script->frames[i] == frame) {
            return i;
        }
    }
    return -1;
}

int sd_script_get_frame_index_at(const sd_script *script, int ticks) {
    if(script == NULL || ticks < 0)
        return -1;

    int pos = 0;
    int next = 0;
    for(int i = 0; i < script->frame_count; i++) {
        next = pos + script->frames[i].tick_len;
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
    int index = sd_script_get_frame_index(script, frame);
    if(index == script->frame_count-1)
        return 1;
    return 0;
}

int sd_script_is_last_frame_at(const sd_script *script, int ticks) {
    const sd_script_frame *frame = sd_script_get_frame_at(script, ticks);
    return sd_script_is_last_frame(script, frame);
}

int sd_script_is_first_frame(const sd_script *script, const sd_script_frame *frame) {
    if(script == NULL)
        return 0;
    int index = sd_script_get_frame_index(script, frame);
    if(index == 0)
        return 1;
    return 0;
}

int sd_script_is_first_frame_at(const sd_script *script, int ticks) {
    const sd_script_frame *frame = sd_script_get_frame_at(script, ticks);
    return sd_script_is_first_frame(script, frame);
}

sd_script_tag* _sd_script_get_tag(const sd_script_frame* frame, const char* tag) {
    for(int i = 0; i < frame->tag_count; i++) {
        if(strcmp(tag, frame->tags[i].key) == 0) {
            return &frame->tags[i];
        }
    }
    return NULL;
}

const sd_script_tag* sd_script_get_tag(const sd_script_frame* frame, const char* tag) {
    if(frame == NULL || tag == NULL) {
        return NULL;
    }
    return _sd_script_get_tag(frame, tag);
}

int sd_script_isset(const sd_script_frame *frame, const char* tag) {
    if(sd_script_get_tag(frame, tag) != NULL) {
        return 1;
    }
    return 0;
}

int sd_script_get(const sd_script_frame *frame, const char* tag) {
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

    int pos = 0;
    int next = 0;
    for(int i = 0; i < script->frame_count; i++) {
        next = pos + script->frames[i].tick_len;
        if(current_tick < pos && sprite_id == script->frames[i].sprite) {
            return i;
        }
        pos = next;
    }

    return -1;
}

int sd_script_next_frame_with_tag(const sd_script *script, const char* tag, int current_tick) {
    if(script == NULL || tag == NULL)
        return -1;
    if(current_tick > sd_script_get_total_ticks(script))
        return -1;

    int pos = 0;
    int next = 0;
    for(int i = 0; i < script->frame_count; i++) {
        next = pos + script->frames[i].tick_len;
        if(current_tick < pos && sd_script_isset(&script->frames[i], tag)) {
            return i;
        }
        pos = next;
    }

    return -1;
}

int sd_script_set_tag(sd_script *script, int frame_id, const char* tag, int value) {
    if(script == NULL || tag == NULL)
        return SD_INVALID_INPUT;
    if(frame_id < 0 || frame_id >= script->frame_count)
        return SD_INVALID_INPUT;

    // Get frame
    sd_script_frame *frame = &script->frames[frame_id];

    // Get tag information
    const char *r_tag;
    const char *r_desc;
    int req_param;
    if(sd_tag_info(tag, &req_param, &r_tag, &r_desc) != SD_SUCCESS) {
        return SD_INVALID_INPUT;
    }

    // See if old tag has been set
    sd_script_tag *old_tag = _sd_script_get_tag(frame, r_tag);
    if(old_tag == NULL) {
        // If the tag does not exist in the frame, set it and its value (if value is required.)
        _create_tag(frame, frame->tag_count);
        frame->tags[frame->tag_count].key = r_tag;
        frame->tags[frame->tag_count].desc = r_desc;
        frame->tags[frame->tag_count].has_param = req_param;
        frame->tags[frame->tag_count].value = value;
        frame->tag_count++;
    } else if(req_param) {
        // If the tag exists and requires a value, set it.
        old_tag->value = value;
        old_tag->has_param = 1;
    }

    return SD_SUCCESS;
}

int sd_script_letter_to_frame(char letter) {
    return (int)(letter - 'A');
}

char sd_script_frame_to_letter(int frame_id) {
    return (char)(frame_id + 'A');
}
