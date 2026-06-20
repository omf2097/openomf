#include "audio/audio.h"
#include "audio/backends/null/null_backend.h"
#include "audio/sound_opts.h"
#include "audio/sound_sources/null_sound_source.h"
#include "common.h"
#include "game/audio/audio_sources.h"
#include "game/audio/playing_sound.h"
#include "game/audio/sound_tracker.h"
#include "utils/log.h"
#include "utils/vector.h"

int sound_tracker_suite_init(void) {
    log_init();
    audio_scan_backends();
    if(!audio_init("NULL", 48000, false, 0, 1.0f, 1.0f)) {
        log_close();
        return 1;
    }
    audio_sources_set_mode(AUDIO_SOURCES_NULL);
    return 0;
}

int sound_tracker_suite_free(void) {
    audio_close();
    audio_sources_set_mode(AUDIO_SOURCES_REAL);
    log_close();
    return 0;
}

void test_play_records_entry(void) {
    null_audio_backend_reset_state();
    sound_tracker t;
    sound_tracker_create(&t);

    sound_tracker_play(&t, 42, false, 5, NULL);

    CU_ASSERT_EQUAL(vector_size(&t.entries), 1);
    const playing_sound *s = vector_get(&t.entries, 0);
    CU_ASSERT_EQUAL(s->tick, 42);
    CU_ASSERT_EQUAL(s->sound_id, 5);
    CU_ASSERT_EQUAL(s->length, (5 + 1) * 1000);
    CU_ASSERT_EQUAL(s->duration, (5 + 1) * 125);
    CU_ASSERT_TRUE(s->playback_id != AUDIO_INVALID_HANDLE);

    CU_ASSERT_EQUAL(null_audio_backend_get_play_count(0), 1);
    CU_ASSERT_EQUAL(null_audio_backend_get_last_sound_id(0), 5);

    sound_tracker_free(&t);
}

void test_play_stores_opts_fields(void) {
    null_audio_backend_reset_state();
    sound_tracker t;
    sound_tracker_create(&t);

    sound_opts opts;
    sound_opts_init(&opts);
    opts.volume = 90;
    opts.panning = -40;
    opts.pitch = 5;
    sound_tracker_play(&t, 0, false, 3, &opts);

    CU_ASSERT_EQUAL(vector_size(&t.entries), 1);
    const playing_sound *s = vector_get(&t.entries, 0);
    CU_ASSERT_EQUAL(s->volume, 90);
    CU_ASSERT_EQUAL(s->panning, -40);
    CU_ASSERT_EQUAL(s->pitch, 5);

    sound_tracker_free(&t);
}

void test_out_of_range_sound_id(void) {
    null_audio_backend_reset_state();
    sound_tracker t;
    sound_tracker_create(&t);

    sound_tracker_play(&t, 0, false, -1, NULL);
    sound_tracker_play(&t, 0, false, 300, NULL);

    CU_ASSERT_EQUAL(vector_size(&t.entries), 0);
    CU_ASSERT_EQUAL(null_audio_backend_get_play_count(0), 0);

    sound_tracker_free(&t);
}

void test_clone_skips_actual_playback(void) {
    null_audio_backend_reset_state();
    sound_tracker t;
    sound_tracker_create(&t);

    sound_tracker_play(&t, 10, true, 3, NULL); // clone=true means no actual play

    CU_ASSERT_EQUAL(vector_size(&t.entries), 1);
    const playing_sound *s = vector_get(&t.entries, 0);
    CU_ASSERT_EQUAL(s->playback_id, AUDIO_INVALID_HANDLE);
    CU_ASSERT_EQUAL(null_audio_backend_get_play_count(0), 0);

    sound_tracker_free(&t);
}

void test_tick_decays_and_expires(void) {
    null_audio_backend_reset_state();
    sound_tracker t;
    sound_tracker_create(&t);

    sound_tracker_play(&t, 0, false, 7, NULL); // duration = 1000ms
    CU_ASSERT_EQUAL(vector_size(&t.entries), 1);

    sound_tracker_tick(&t, 999, NULL, NULL);
    CU_ASSERT_EQUAL(vector_size(&t.entries), 1);

    sound_tracker_tick(&t, 1, NULL, NULL);
    CU_ASSERT_EQUAL(vector_size(&t.entries), 0);

    sound_tracker_free(&t);
}

void test_duration_math_per_id(void) {
    null_audio_backend_reset_state();
    sound_tracker t;
    sound_tracker_create(&t);

    for(int id = 0; id < 4; id++) {
        sound_tracker_play(&t, id, false, id, NULL);
    }

    for(int i = 0; i < 4; i++) {
        const playing_sound *s = vector_get(&t.entries, i);
        CU_ASSERT_EQUAL(s->duration, (s->sound_id + 1) * 125);
        CU_ASSERT_TRUE(s->duration > 0);
    }

    sound_tracker_free(&t);
}

void test_clone_copies_entries_independently(void) {
    null_audio_backend_reset_state();
    sound_tracker a, b;
    sound_tracker_create(&a);
    sound_tracker_create(&b);

    sound_tracker_play(&a, 1, false, 0, NULL); // 125ms
    sound_tracker_play(&a, 2, false, 1, NULL); // 250ms
    sound_tracker_clone(&b, &a);

    CU_ASSERT_EQUAL(vector_size(&a.entries), 2);
    CU_ASSERT_EQUAL(vector_size(&b.entries), 2);

    // Mutate a; b should remain intact.
    sound_tracker_tick(&a, 200, NULL, NULL); // drops sound_id=0 (duration 125)
    CU_ASSERT_EQUAL(vector_size(&a.entries), 1);
    CU_ASSERT_EQUAL(vector_size(&b.entries), 2);

    sound_tracker_free(&a);
    sound_tracker_free(&b);
}

void test_merge_same_in_both_is_noop(void) {
    null_audio_backend_reset_state();
    sound_tracker old_t, new_t;
    sound_tracker_create(&old_t);
    sound_tracker_create(&new_t);

    sound_tracker_play(&old_t, 5, false, 0, NULL);
    sound_tracker_clone(&new_t, &old_t);

    const int plays_before = null_audio_backend_get_play_count(0);
    sound_tracker_merge(&old_t, &new_t);
    CU_ASSERT_EQUAL(null_audio_backend_get_fade_count(0), 0);
    CU_ASSERT_EQUAL(null_audio_backend_get_play_count(0), plays_before);

    sound_tracker_free(&old_t);
    sound_tracker_free(&new_t);
}

void test_merge_old_only_fades_out(void) {
    null_audio_backend_reset_state();
    sound_tracker old_t, new_t;
    sound_tracker_create(&old_t);
    sound_tracker_create(&new_t);

    sound_tracker_play(&old_t, 5, false, 0, NULL); // playback on channel 0
    // new_t is empty: old entry was rolled away.

    sound_tracker_merge(&old_t, &new_t);
    CU_ASSERT_EQUAL(null_audio_backend_get_fade_count(0), 1);

    sound_tracker_free(&old_t);
    sound_tracker_free(&new_t);
}

void test_merge_new_only_starts_playback(void) {
    null_audio_backend_reset_state();
    sound_tracker old_t, new_t;
    sound_tracker_create(&old_t);
    sound_tracker_create(&new_t);

    // Record an unplayed entry on the new side (clone=true means no playback handle).
    sound_tracker_play(&new_t, 7, true, 0, NULL);
    CU_ASSERT_EQUAL(null_audio_backend_get_play_count(0), 0);

    sound_tracker_merge(&old_t, &new_t);
    CU_ASSERT_EQUAL(null_audio_backend_get_play_count(0), 1);
    CU_ASSERT_EQUAL(null_audio_backend_get_last_sound_id(0), 0);

    sound_tracker_free(&old_t);
    sound_tracker_free(&new_t);
}

void test_eviction_when_all_channels_busy(void) {
    null_audio_backend_reset_state();
    sound_tracker t;
    sound_tracker_create(&t);

    // Fill all channels with default-priority (10) sounds.
    for(int ch = 0; ch < 3; ch++) {
        sound_tracker_play(&t, ch, false, ch, NULL);
    }
    for(int ch = 0; ch < 3; ch++) {
        CU_ASSERT_EQUAL(null_audio_backend_get_play_count(ch), 1);
        CU_ASSERT_EQUAL(null_audio_backend_get_stop_count(ch), 0);
    }

    // A new play with default priority (equal to existing) evicts the first channel.
    sound_tracker_play(&t, 99, false, 4, NULL);
    CU_ASSERT_EQUAL(null_audio_backend_get_stop_count(0), 1);
    CU_ASSERT_EQUAL(null_audio_backend_get_play_count(0), 2);
    CU_ASSERT_EQUAL(null_audio_backend_get_last_sound_id(0), 4);

    sound_tracker_free(&t);
}

void test_lower_priority_cannot_evict(void) {
    null_audio_backend_reset_state();
    sound_tracker t;
    sound_tracker_create(&t);

    sound_opts hi;
    sound_opts_init(&hi);
    hi.priority = 100;
    for(int ch = 0; ch < 3; ch++) {
        sound_tracker_play(&t, ch, false, ch, &hi);
    }

    // A lower-priority play finds no free channel and no evictable channel.
    sound_opts lo;
    sound_opts_init(&lo);
    lo.priority = 5;
    const unsigned int entries_before = vector_size(&t.entries);
    sound_tracker_play(&t, 99, false, 4, &lo);
    CU_ASSERT_EQUAL(vector_size(&t.entries), entries_before); // no new entry recorded
    for(int ch = 0; ch < 3; ch++) {
        CU_ASSERT_EQUAL(null_audio_backend_get_stop_count(ch), 0);
    }

    sound_tracker_free(&t);
}

void test_skip_duplicate_drops_second_play(void) {
    null_audio_backend_reset_state();
    sound_tracker t;
    sound_tracker_create(&t);

    sound_tracker_play(&t, 1, false, 5, NULL);
    CU_ASSERT_EQUAL(null_audio_backend_get_play_count(0), 1);

    sound_opts skip;
    sound_opts_init(&skip);
    skip.skip_duplicate = true;
    sound_tracker_play(&t, 2, false, 5, &skip);
    CU_ASSERT_EQUAL(null_audio_backend_get_play_count(0), 1); // unchanged
    CU_ASSERT_EQUAL(vector_size(&t.entries), 1);              // no second entry

    sound_tracker_free(&t);
}

static int stub_pan_lookup_constant(void *ctx, uint32_t object_id) {
    (void)object_id;
    return *(int *)ctx;
}

void test_pan_sweep_interpolation(void) {
    null_audio_backend_reset_state();
    sound_tracker t;
    sound_tracker_create(&t);

    sound_opts opts;
    sound_opts_init(&opts);
    opts.panning = -50;
    opts.panning_end = 50;
    opts.has_panning_sweep = true;
    sound_tracker_play(&t, 0, false, 7, &opts); // 1000ms total
    CU_ASSERT_EQUAL(null_audio_backend_get_last_pan(0), -50);
    CU_ASSERT_EQUAL(null_audio_backend_get_pan_update_count(0), 0);

    sound_tracker_tick(&t, 500, NULL, NULL);

    // At the midpoint, pan should be roughly 0 (allow a couple units of integer rounding).
    const int mid_pan = null_audio_backend_get_last_pan(0);
    CU_ASSERT_TRUE(mid_pan >= -2 && mid_pan <= 2);
    CU_ASSERT_EQUAL(null_audio_backend_get_pan_update_count(0), 1);

    sound_tracker_free(&t);
}

void test_follow_calls_lookup(void) {
    null_audio_backend_reset_state();
    sound_tracker t;
    sound_tracker_create(&t);

    sound_opts opts;
    sound_opts_init(&opts);
    opts.panning = 0;
    opts.follow_object_id = 42;
    sound_tracker_play(&t, 0, false, 5, &opts);
    CU_ASSERT_EQUAL(null_audio_backend_get_pan_update_count(0), 0);

    int desired_pan = 75;
    sound_tracker_tick(&t, 0, stub_pan_lookup_constant, &desired_pan);
    CU_ASSERT_EQUAL(null_audio_backend_get_last_pan(0), 75);
    CU_ASSERT_EQUAL(null_audio_backend_get_pan_update_count(0), 1);

    // Same value again -- no redundant backend call.
    sound_tracker_tick(&t, 0, stub_pan_lookup_constant, &desired_pan);
    CU_ASSERT_EQUAL(null_audio_backend_get_pan_update_count(0), 1);

    sound_tracker_free(&t);
}

void test_stop_duplicate_replaces_first_play(void) {
    null_audio_backend_reset_state();
    sound_tracker t;
    sound_tracker_create(&t);

    sound_tracker_play(&t, 1, false, 5, NULL);
    CU_ASSERT_EQUAL(null_audio_backend_get_play_count(0), 1);
    CU_ASSERT_EQUAL(null_audio_backend_get_stop_count(0), 0);

    sound_opts stop;
    sound_opts_init(&stop);
    stop.stop_duplicate = true;
    sound_tracker_play(&t, 2, false, 5, &stop);
    CU_ASSERT_EQUAL(null_audio_backend_get_play_count(0), 2);
    CU_ASSERT_EQUAL(null_audio_backend_get_stop_count(0), 1);

    sound_tracker_free(&t);
}

void test_stale_handle_is_ignored(void) {
    null_audio_backend_reset_state();
    sound_source a, b;
    null_sound_source_load(&a, 1);
    null_sound_source_load(&b, 2);

    sound_opts opts;
    sound_opts_init(&opts);
    opts.channel = 0;

    const uint32_t h_old = audio_play_source(&a, &opts); // ch0, first guid
    const uint32_t h_new = audio_play_source(&b, &opts); // forced replace: ch0, next guid
    CU_ASSERT_TRUE(h_old != h_new);

    // stale handle should not affect anything
    audio_set_pan(h_old, 50);
    CU_ASSERT_EQUAL(null_audio_backend_get_pan_update_count(0), 0);
    audio_fade_out(h_old, 500);
    CU_ASSERT_EQUAL(null_audio_backend_get_fade_count(0), 0);

    // Current handle should still work
    audio_set_pan(h_new, 50);
    CU_ASSERT_EQUAL(null_audio_backend_get_pan_update_count(0), 1);
}

void sound_tracker_test_suite(CU_pSuite suite) {
    ADD_TEST("Test play records entry", test_play_records_entry);
    ADD_TEST("Test play stores opts vol/pan/pitch", test_play_stores_opts_fields);
    ADD_TEST("Test out-of-range sound_id is rejected", test_out_of_range_sound_id);
    ADD_TEST("Test clone-mode skips actual playback", test_clone_skips_actual_playback);
    ADD_TEST("Test tick decays and expires entries", test_tick_decays_and_expires);
    ADD_TEST("Test duration math per id", test_duration_math_per_id);
    ADD_TEST("Test clone copies entries independently", test_clone_copies_entries_independently);
    ADD_TEST("Test merge: identical entries are a no-op", test_merge_same_in_both_is_noop);
    ADD_TEST("Test merge: old-only entries fade out", test_merge_old_only_fades_out);
    ADD_TEST("Test merge: new-only entries start playback", test_merge_new_only_starts_playback);
    ADD_TEST("Test eviction when all channels are busy", test_eviction_when_all_channels_busy);
    ADD_TEST("Test lower priority cannot evict", test_lower_priority_cannot_evict);
    ADD_TEST("Test skip_duplicate drops second play", test_skip_duplicate_drops_second_play);
    ADD_TEST("Test stop_duplicate replaces first play", test_stop_duplicate_replaces_first_play);
    ADD_TEST("Test pan sweep interpolation", test_pan_sweep_interpolation);
    ADD_TEST("Test follow calls lookup", test_follow_calls_lookup);
    ADD_TEST("Test stale handle is ignored", test_stale_handle_is_ignored);
}
