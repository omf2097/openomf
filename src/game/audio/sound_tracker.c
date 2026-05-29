#include "game/audio/sound_tracker.h"
#include "audio/audio.h"
#include "game/audio/audio_sources.h"
#include "game/audio/playing_sound.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "utils/vector.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>

void sound_tracker_create(sound_tracker *t) {
    vector_create(&t->entries, sizeof(playing_sound));
}

void sound_tracker_free(sound_tracker *t) {
    vector_free(&t->entries);
}

void sound_tracker_clone(sound_tracker *dst, const sound_tracker *src) {
    iterator it;
    vector_iter_begin((vector *)&src->entries, &it);
    playing_sound *s;
    foreach(it, s) {
        vector_append(&dst->entries, s);
    }
}

static void sound_tracker_update_pans(sound_tracker *t, sound_pan_lookup lookup, void *ctx) {
    iterator it;
    vector_iter_begin(&t->entries, &it);
    playing_sound *s;
    foreach(it, s) {
        if(s->playback_id < 0) {
            continue;
        }
        int new_pan;
        if(s->follow_object_id != 0 && lookup != NULL) {
            // Object tracking has priority over sweep
            const int looked_up = lookup(ctx, s->follow_object_id);
            if(looked_up == INT_MIN) {
                continue; // source object gone -- keep last pan
            }
            new_pan = looked_up;
        } else if(s->has_pan_sweep && s->total_duration_ms > 0) {
            // Sweep enabled; slide from start to target.
            const int elapsed_ms = s->total_duration_ms - s->duration;
            new_pan = s->pan_start + (elapsed_ms * (s->pan_end - s->pan_start)) / s->total_duration_ms;
        } else {
            // Nothing to do for this sound, just bail out.
            continue;
        }

        // set the new panning value if there was a change.
        new_pan = clamp(new_pan, -100, 100);
        if(new_pan != s->panning) {
            s->panning = new_pan;
            audio_set_pan(s->playback_id, new_pan);
        }
    }
}

void sound_tracker_tick(sound_tracker *t, const int ms_elapsed, sound_pan_lookup lookup, void *ctx) {
    iterator it;
    vector_iter_begin(&t->entries, &it);
    playing_sound *s;
    foreach(it, s) {
        s->duration -= ms_elapsed;
        if(s->duration <= 0) {
            vector_delete(&t->entries, &it);
        }
    }
    sound_tracker_update_pans(t, lookup, ctx);
}

void sound_tracker_merge(sound_tracker *old, sound_tracker *new) {
    // We need to do several things here:
    // * Leave any sounds that are playing in both states alone
    // * Fade out any sounds only playing in the old state
    // * Fade in any new sounds, and start playing them at the appropriate offset

    iterator it, it2;
    playing_sound *s, *s2;

    // Fade out sounds present only in old.
    vector_iter_begin(&old->entries, &it);
    foreach(it, s) {
        bool found = false;
        vector_iter_begin(&new->entries, &it2);
        foreach(it2, s2) {
            if(s->sound_id == s2->sound_id && s->tick == s2->tick) {
                // same sound, same frame
                found = true;
                break;
            }
        }
        if(!found) {
            // this sound no longer exists after a rollback, so we need to fade it out
            audio_fade_out(s->playback_id, 500);
            // don't bother adding it to the new sound vector though
        }
    }

    // Start playback at the appropriate offset for sounds present only in new.
    vector_iter_begin(&new->entries, &it);
    foreach(it, s) {
        bool found = false;
        vector_iter_begin(&old->entries, &it2);
        foreach(it2, s2) {
            if(s->sound_id == s2->sound_id && s->tick == s2->tick) {
                found = true;
                break;
            }
        }

        if(!found) {
            // this sound was added during the rollback, so we need to start playing it,
            // but we need to determine the playback offset AND fade it in
            //
            // this sound should NOT have been played already!
            assert(s->playback_id == -1);

            sound_source src;
            if(!sound_source_pick(&src, s->sound_id)) {
                log_error("Requested sound sample %d not found or empty", s->sound_id);
                return;
            }

            // calculate the offset into the buffer we need
            const int effective_freq = pitched_samplerate(src.freq, s->pitch);
            const int total_duration = (int)(s->length * 1000 / effective_freq);
            const int elapsed_ms = total_duration - s->duration;
            const int offset = elapsed_ms * effective_freq / 1000;

            log_debug(
                "Playing sound %d with pitch %d added after rollback at tick %d otf length %zu at offset %d (duration "
                "total %d, remaining %d)",
                s->sound_id, s->pitch, s->tick, src.len, offset, total_duration, s->duration);

            // guard against playing beyond the end of the buffer
            if((size_t)offset < src.len) {
                src.buf += offset;
                src.len -= offset;
                sound_opts opts;
                sound_opts_init(&opts);
                opts.volume = s->volume;
                opts.panning = s->panning;
                opts.pitch = s->pitch;
                opts.fade_in_ms = 500; // TODO decide on a fade in time
                s->playback_id = audio_play_source(&src, &opts);
            }
            sound_source_close(&src);
        }
    }
}

void sound_tracker_play(sound_tracker *t, const int tick, const bool clone, const int sound_id,
                        const sound_opts *opts) {
    if(sound_id < 0 || sound_id > 299) {
        return;
    }

    sound_source src;
    if(!sound_source_pick(&src, sound_id)) {
        log_error("Requested sound sample %d not found or empty", sound_id);
        return;
    }

    sound_opts defaults;
    if(opts == NULL) {
        sound_opts_init(&defaults);
        opts = &defaults;
    }
    const int effective_freq = pitched_samplerate(src.freq, opts->pitch);

    playing_sound s;
    s.tick = tick;
    s.sound_id = sound_id;
    s.length = (int)src.len;
    s.duration = (int)(src.len * 1000 / effective_freq);
    s.total_duration_ms = s.duration;
    s.volume = opts->volume;
    s.panning = opts->panning;
    s.pan_start = opts->panning;
    s.pan_end = opts->panning_end;
    s.has_pan_sweep = opts->has_panning_sweep;
    s.follow_object_id = opts->follow_object_id;
    s.pitch = opts->pitch;
    s.playback_id = -1;

    if(!clone) {
        // cloned game states reach here only to record the entry -- never actually play.
        s.playback_id = audio_play_source(&src, opts);
        if(s.playback_id == -1) {
            sound_source_close(&src);
            return;
        }
    }
    sound_source_close(&src);

    vector_append(&t->entries, &s);
}
