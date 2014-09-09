#include "game/scenes/mainmenu/menu_video.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"

struct resolution_t {
    int w;  int h;  const char *name;
} _resolutions[] = {
    {320,   200,    "320x200"},
    {640,   400,    "640x400"},
    {800,   480,    "800x480"},
    {800,   600,    "800x600"},
    {1024,  768,    "1024x768"},
    {1280,  720,    "1280x720"},
    {1280,  800,    "1280x800"},
    {1280,  960,    "1280x960"},
    {1280,  1024,   "1280x1024"},
    {1366,  768,    "1366x768"},
    {1440,  900,    "1440x900"},
    {1600,  1000,   "1600x1000"},
    {1600,  1200,   "1600x1200"},
    {1650,  1080,   "1650x1080"},
    {1920,  1080,   "1920x1080"},
    {1920,  1200,   "1920x1200"},
    {2560,  1440,   "2560x1440"},
    {2560,  1600,   "2560x1600"},
    {3840,  2160,   "3840x2160"},
};

typedef struct resolution_t resolution;

typedef struct {
    component video_header;
    component resolution_toggle;
    component vsync_toggle;
    component fullscreen_toggle;
    component scaler_toggle;
    component scale_factor_toggle;
    component video_done_button;
} video_menu_data;

resolution *find_resolution_by_settings(settings *s) {
    int w = s->video.screen_w;
    int h = s->video.screen_h;

    for(int i = 0;i < sizeof(_resolutions)/sizeof(resolution);i++) {
        if(w == _resolutions[i].w && h == _resolutions[i].h) {
            return &_resolutions[i];
        }
    }
    return NULL;
}

void video_done_clicked(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    settings_video *v = &settings_get()->video;
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync, v->scaler, v->scale_factor);
    mainmenu_prev_menu(c, userdata);

    if(local->old_video_settings.screen_w != v->screen_w ||
        local->old_video_settings.screen_h != v->screen_h ||
        local->old_video_settings.fullscreen != v->fullscreen) {
        // Resolution confirmation dialog
        mainmenu_enter_menu_video_confirm(c, userdata);
        time(&local->video_accept_timer);
        local->video_accept_secs = 20;
        if(sprintf(local->video_accept_label,
                   "ACCEPT NEW RESOLUTION? %d",
                   local->video_accept_secs) > 0) {
            ((textbutton*)local->video_confirm_header.obj)->text = local->video_accept_label;
        }
    }
}

void video_confirm_cancel_clicked(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    settings_video *v = &settings_get()->video;
    *v = local->old_video_settings;
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync, v->scaler, v->scale_factor);
    mainmenu_prev_menu(c, userdata);
}

void resolution_toggled(component *c, void *userdata, int pos) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    settings_video *v = &settings_get()->video;
    if(local->is_custom_resolution) {
        // The first index is always the custom resolution
        if(pos == 0) {
            v->screen_w = local->custom_resolution.x;
            v->screen_h = local->custom_resolution.y;
        } else {
            v->screen_w = _resolutions[pos-1].w;
            v->screen_h = _resolutions[pos-1].h;
        }
    } else {
        v->screen_w = _resolutions[pos].w;
        v->screen_h = _resolutions[pos].h;
    }
}

void scaler_toggled(component *c, void *userdata, int pos) {
    mainmenu_local *local = userdata;
    settings_video *v = &settings_get()->video;

    // Set scaler
    v->scaler = realloc(v->scaler, strlen(textselector_get_current_text(c))+1);
    strcpy(v->scaler, textselector_get_current_text(c));

    // If scaler is NEAREST, set factor to 1 and disable
    if(textselector_get_pos(c) == 0) {
        textselector_clear_options(&local->scale_factor_toggle);
        textselector_add_option(&local->scale_factor_toggle, "1");
        local->scale_factor_toggle.disabled = 1;
        v->scale_factor = 1;
    } else {
        local->scale_factor_toggle.disabled = 0;

        int *list;
        scaler_plugin scaler;
        scaler_init(&scaler);
        plugins_get_scaler(&scaler, v->scaler);
        int len = scaler_get_factors_list(&scaler, &list);
        textselector_clear_options(&local->scale_factor_toggle);
        for(int i = 0; i < len; i++) {
            sprintf(local->scaling_factor_labels[i], "%d", list[i]);
            textselector_add_option(&local->scale_factor_toggle, local->scaling_factor_labels[i]);
        }

        // Select first scale factor from the list
        v->scale_factor = list[0];
    }

    // Always select first factor option if scaler has changed.
    textselector_set_pos(&local->scale_factor_toggle, 0);

    // If scaler is "Nearest", disable factor toggle
    local->scale_factor_toggle.disabled = (textselector_get_pos(c) == 0);

    // Reinig after algorithm change
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync, v->scaler, v->scale_factor);
}

void scaling_factor_toggled(component *c, void *userdata, int pos) {
    //mainmenu_local *local = userdata;
    settings_video *v = &settings_get()->video;

    int *list;
    scaler_plugin scaler;
    scaler_init(&scaler);
    plugins_get_scaler(&scaler, v->scaler);
    scaler_get_factors_list(&scaler, &list);
    v->scale_factor = list[pos];

    // Reinit after factor change
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync, v->scaler, v->scale_factor);
}

void menu_video_free(menu *menu) {
    video_menu_data *local = menu_get_userdata(menu);
    textbutton_free(&local->video_header);
    textselector_free(&local->resolution_toggle);
    textselector_free(&local->vsync_toggle);
    textselector_free(&local->fullscreen_toggle);
    textselector_free(&local->scale_factor_toggle);
    textselector_free(&local->scaler_toggle);
    textbutton_free(&local->video_done_button);
    free(local);
}

void menu_video_create(menu *menu) {
    video_menu_data *local = malloc(sizeof(video_menu_data));





    menu_set_userdata(local);
    menu_set_free_cb(menu_video_free);
}