#include "formats/script_reader.h"

void script_reader_load(script_reader *r, const script *script) {
    r->script = script; // Note -- the script is saved elsewhere, we just reference!
    r->frame = NULL;
    r->frame_index = -1;
    r->frame_start = 0;
    r->frame_end = 0;
    r->frame_valid = false;
    script_reader_reset(r);
}

void script_reader_reset(script_reader *r) {
    r->tick = 0;
    r->previous_tick = ~0u;
}

void script_reader_seek(script_reader *r, uint32_t tick) {
    r->tick = tick;
}

void script_reader_advance(script_reader *r, int n) {
    r->tick += n;
}

void script_reader_mark_previous(script_reader *r) {
    r->previous_tick = r->tick;
}

void script_reader_mark_entered(script_reader *r) {
    r->previous_tick = r->tick - 1;
}

uint32_t script_reader_tick(const script_reader *r) {
    return r->tick;
}

const script *script_reader_get_script(const script_reader *r) {
    return r->script;
}

static void resolve(script_reader *r) {
    // No script set, so nothing to do; bail out.
    if(r->script == NULL) {
        r->frame = NULL;
        r->frame_index = -1;
        r->frame_valid = false;
        return;
    }

    // Forward fast path; we just step into the next frame -- no need to scan.
    if(r->frame_valid && r->tick == r->frame_end) {
        const int next_index = r->frame_index + 1;
        const script_frame *next = script_get_frame(r->script, next_index);
        if(next != NULL) {
            const uint32_t end = r->frame_end + (uint32_t)next->tick_len;
            if(r->tick < end) {
                r->frame = next;
                r->frame_index = next_index;
                r->frame_start = r->frame_end;
                r->frame_end = end;
                return;
            }
        }
    }

    // Slow path -- full scan from the beginning.
    uint32_t pos = 0;
    for(int i = 0; i < script_get_frame_count(r->script); i++) {
        const script_frame *frame = script_get_frame(r->script, i);
        const uint32_t next = pos + (uint32_t)frame->tick_len;
        if(r->tick >= pos && r->tick < next) {
            r->frame = frame;
            r->frame_index = i;
            r->frame_start = pos;
            r->frame_end = next;
            r->frame_valid = true;
            return;
        }
        pos = next;
    }

    // The position is over the end of the animation. We cache that as [pos ... UINT32_MAX].
    r->frame = NULL;
    r->frame_index = -1;
    r->frame_start = pos;
    r->frame_end = UINT32_MAX;
    r->frame_valid = true;
}

const script_frame *script_reader_frame(const script_reader *r) {
    // Okay, so we cast away the const here. Hear me out -- this is fine. We don't change the playback state, only
    // the cache which is implementation detail. Nothing to see here!
    script_reader *m = (script_reader *)r;
    if(!(m->frame_valid && m->tick >= m->frame_start && m->tick < m->frame_end)) {
        // Cache is not valid; refresh.
        resolve(m);
    }
    return m->frame;
}

bool script_reader_isset(const script_reader *r, script_tag tag) {
    return script_get_tag_by_id(script_reader_frame(r), tag) != NULL;
}

int script_reader_get(const script_reader *r, script_tag tag) {
    return script_get_tag_value_by_id(script_reader_frame(r), tag);
}

bool script_reader_frame_changed(const script_reader *r) {
    return script_frame_changed(r->script, r->previous_tick, r->tick);
}
