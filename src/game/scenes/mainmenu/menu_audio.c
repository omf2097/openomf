#include "game/scenes/mainmenu/menu_audio.h"
#include "audio/audio.h"
#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <stdio.h>

typedef struct {
    settings_sound old_audio_settings;
    component *lib_selector;
    component *resampler_selector;
    component *freq_selector;
} audio_menu_data;

static const char *mono_opts[] = {"OFF", "ON"};

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
    if(s->music_frequency != local->old_audio_settings.music_frequency ||
       s->music_resampler != local->old_audio_settings.music_resampler ||
       s->music_mono != local->old_audio_settings.music_mono) {
        audio_close();
        if(audio_init(s->music_frequency, s->music_mono, s->music_resampler, s->music_vol / 10.0f,
                      s->sound_vol / 10.0f)) {
            audio_play_music(PSM_MENU);
        }
    }
}

void menu_audio_reset_freqs(audio_menu_data *local, int use_settings) {
    const audio_freq *freqs = audio_get_freqs();
    textselector_clear_options(local->freq_selector);
    for(int i = 0; freqs[i].name != 0; i++) {
        textselector_add_option(local->freq_selector, freqs[i].name);
        int use = (use_settings) ? (settings_get()->sound.music_frequency == freqs[i].freq) : freqs[i].is_default;
        if(use) {
            textselector_set_pos(local->freq_selector, i);
        }
    }
}

void menu_audio_reset_resamplers(audio_menu_data *local, int use_settings) {
    const audio_mod_resampler *resamplers = audio_get_resamplers();
    textselector_clear_options(local->resampler_selector);
    for(int i = 0; resamplers[i].name != 0; i++) {
        textselector_add_option(local->resampler_selector, resamplers[i].name);
        int use = (use_settings) ? (settings_get()->sound.music_resampler == resamplers[i].internal_id)
                                 : resamplers[i].is_default;
        if(use) {
            textselector_set_pos(local->resampler_selector, i);
        }
    }
}

void menu_audio_freq_toggled(component *c, void *userdata, int pos) {
    const audio_freq *freqs = audio_get_freqs();
    settings_get()->sound.music_frequency = freqs[pos].freq;
}

void menu_audio_resampler_toggled(component *c, void *userdata, int pos) {
    const audio_mod_resampler *resamplers = audio_get_resamplers();
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

    // Text config
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_BIG;
    tconf.halign = TEXT_CENTER;
    tconf.cforeground = TEXT_BRIGHT_GREEN;

    // Create menu and its header
    component *menu = menu_create(11);
    menu_attach(menu, label_create(&tconf, "AUDIO"));
    menu_attach(menu, filler_create());
    menu_attach(menu,
                textslider_create_bind(&tconf, "SOUND",
                                       "Raise or lower the volume of all sound effects. Press right or left to change.",
                                       10, 1, menu_audio_sound_slide, NULL, &settings_get()->sound.sound_vol));
    menu_attach(menu, textslider_create_bind(&tconf, "MUSIC",
                                             "Raise or lower the volume of music. Press right or left to change.", 10,
                                             1, menu_audio_music_slide, NULL, &settings_get()->sound.music_vol));
    menu_attach(menu, textselector_create_bind_opts(&tconf, "MONO", NULL, NULL, NULL, &settings_get()->sound.music_mono,
                                                    mono_opts, 2));
    local->freq_selector = textselector_create(&tconf, "FREQUENCY:", NULL, menu_audio_freq_toggled, local);
    menu_audio_reset_freqs(local, 1);
    menu_attach(menu, local->freq_selector);

    local->resampler_selector = textselector_create(&tconf, "RESAMPLE:", NULL, menu_audio_resampler_toggled, local);
    menu_audio_reset_resamplers(local, 1);
    menu_attach(menu, local->resampler_selector);

    menu_attach(menu, textbutton_create(&tconf, "DONE", "Exit from this menu.", COM_ENABLED, menu_audio_done, local));

    // Userdata & free function for it
    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_audio_free);
    return menu;
}
