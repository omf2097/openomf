#include "game/scenes/mainmenu/menu_audio.h"
#include "audio/audio.h"
#include "audio/sources/psm_source.h"
#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "utils/allocator.h"
#include "utils/log.h"

typedef struct {
    settings_sound old_audio_settings;
    component *lib_selector;
    component *resampler_selector;
    component *freq_selector;
} audio_menu_data;

static const char *mono_opts[] = {"OFF", "ON"};

const char *music_type_names[] = {
    "ORIGINALS", "REMIXES", "BOTH"
};


void menu_audio_music_slide(component *c, void *userdata, int pos) {
    audio_set_music_volume(pos / 10.0f);
}

void menu_audio_sound_slide(component *c, void *userdata, int pos) {
    audio_set_sound_volume(pos / 10.0f);
}

void menu_audio_done(component *c, void *userdata) {
    audio_menu_data *local = userdata;

    // Set menu as finished
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;

    // Reload music if changes made
    settings_sound *s = &settings_get()->sound;
    if(s->sample_rate != local->old_audio_settings.sample_rate ||
       s->music_resampler != local->old_audio_settings.music_resampler ||
       s->music_mono != local->old_audio_settings.music_mono) {
        audio_close();
        if(audio_init(s->player, s->sample_rate, s->music_mono, s->music_resampler, s->music_vol / 10.0f,
                      s->sound_vol / 10.0f)) {
            audio_play_music(PSM_MENU);
        }
    }
}

void menu_audio_reset_freqs(audio_menu_data *local, int use_settings) {
    const audio_sample_rate *sample_rates;
    unsigned sample_rate_count = audio_get_sample_rates(&sample_rates);
    textselector_clear_options(local->freq_selector);
    for(unsigned i = 0; i < sample_rate_count; i++) {
        textselector_add_option(local->freq_selector, sample_rates[i].name);
        int use = (use_settings) ? (settings_get()->sound.sample_rate == sample_rates[i].sample_rate)
                                 : sample_rates[i].is_default;
        if(use) {
            textselector_set_pos(local->freq_selector, i);
        }
    }
}

void menu_audio_reset_resamplers(audio_menu_data *local, int use_settings) {
    const music_resampler *resamplers;
    unsigned resampler_count = psm_get_resamplers(&resamplers);
    textselector_clear_options(local->resampler_selector);
    for(unsigned i = 0; i < resampler_count; i++) {
        textselector_add_option(local->resampler_selector, resamplers[i].name);
        int use = (use_settings) ? (settings_get()->sound.music_resampler == resamplers[i].internal_id)
                                 : resamplers[i].is_default;
        if(use) {
            textselector_set_pos(local->resampler_selector, i);
        }
    }
}

void menu_audio_freq_toggled(component *c, void *userdata, int pos) {
    const audio_sample_rate *sample_rates;
    audio_get_sample_rates(&sample_rates);
    settings_get()->sound.sample_rate = sample_rates[pos].sample_rate;
}

void menu_audio_resampler_toggled(component *c, void *userdata, int pos) {
    const music_resampler *resamplers;
    psm_get_resamplers(&resamplers);
    settings_get()->sound.music_resampler = resamplers[pos].internal_id;
}

void menu_audio_free(component *c) {
    audio_menu_data *local = menu_get_userdata(c);
    omf_free(local);
    menu_set_userdata(c, local);
}

component *menu_audio_create(scene *s) {
    // Menu userdata
    audio_menu_data *local = omf_calloc(1, sizeof(audio_menu_data));
    local->old_audio_settings = settings_get()->sound;

    // Create menu and its header
    component *menu = menu_create();

    menu_attach(menu, label_create_title("AUDIO"));
    menu_attach(menu, filler_create());
    component *volume_textslider = textslider_create_bind(
        "SOUND", "Raise or lower the volume of all sound effects. Press right or left to change.", 10, 1,
        menu_audio_sound_slide, NULL, &settings_get()->sound.sound_vol);
    textslider_disable_panning(volume_textslider);
    menu_attach(menu, volume_textslider);
    menu_attach(menu,
                textslider_create_bind("MUSIC", "Raise or lower the volume of music. Press right or left to change.",
                                       10, 1, menu_audio_music_slide, NULL, &settings_get()->sound.music_vol));
    menu_attach(
        menu, textselector_create_bind_opts("MONO", NULL, NULL, NULL, &settings_get()->sound.music_mono, mono_opts, 2));
    local->freq_selector = textselector_create("FREQUENCY:", NULL, menu_audio_freq_toggled, local);
    menu_audio_reset_freqs(local, 1);
    menu_attach(menu, local->freq_selector);

    local->resampler_selector = textselector_create("RESAMPLE:", NULL, menu_audio_resampler_toggled, local);
    menu_audio_reset_resamplers(local, 1);
    menu_attach(menu, local->resampler_selector);

    menu_attach(menu,
                textselector_create_bind_opts("MUSIC:",
                                              "Which type of game music to use."
                                              "Whenever more than one track is available for a given scene a random one will be chosen.",
                                              NULL, NULL, &settings_get()->sound.music_type, music_type_names,
                                              3));


    menu_attach(menu, button_create("DONE", "Exit from this menu.", false, false, menu_audio_done, local));

    // Userdata & free function for it
    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_audio_free);
    return menu;
}
