#include <stdio.h>

#include "game/scenes/mainmenu/menu_video.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"
#include "game/menu/label.h"
#include "game/menu/filler.h"
#include "game/menu/sizer.h"

#include "game/utils/settings.h"
#include "plugins/plugins.h"

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
    vec2i custom_resolution;
    int is_custom_resolution;
    component *scaler;
    component *factor;
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

void resolution_toggled(component *c, void *userdata, int pos) {
    video_menu_data *local = userdata;
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

/*
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
*/
void scaler_toggled(component *c, void *userdata, int pos) {
    video_menu_data *local = userdata;
    settings_video *v = &settings_get()->video;

    // Set scaler
    v->scaler = realloc(v->scaler, strlen(textselector_get_current_text(c))+1);
    strcpy(v->scaler, textselector_get_current_text(c));

    // If scaler is NEAREST, set factor to 1 and disable
    char tmp_buf[32];
    if(textselector_get_pos(c) == 0) {
        textselector_clear_options(local->factor);
        textselector_add_option(local->factor, "1");
        component_disable(local->factor, 1);
        v->scale_factor = 1;
    } else {
        component_disable(local->factor, 0);

        int *list;
        scaler_plugin scaler;
        scaler_init(&scaler);
        plugins_get_scaler(&scaler, v->scaler);
        int len = scaler_get_factors_list(&scaler, &list);
        textselector_clear_options(local->factor);
        for(int i = 0; i < len; i++) {
            sprintf(tmp_buf, "%d", list[i]);
            textselector_add_option(local->factor, tmp_buf);
        }

        // Select first scale factor from the list
        v->scale_factor = list[0];
    }

    // Always select first factor option if scaler has changed.
    textselector_set_pos(local->factor, 0);

    // If scaler is "Nearest", disable factor toggle
    component_disable(local->factor, (textselector_get_pos(c) == 0));

    // Reinig after algorithm change
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync, v->scaler, v->scale_factor);
}

void scaling_factor_toggled(component *c, void *userdata, int pos) {
    //video_menu_data *local = userdata;
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


void menu_video_done(component *c, void *u) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
    settings_video *v = &settings_get()->video;
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync, v->scaler, v->scale_factor);
}

void menu_video_free(component *c) {
    video_menu_data *local = menu_get_userdata(c);
    free(local);
}

component* menu_video_create(scene *s) {
    // Menu userdata
    video_menu_data *local = malloc(sizeof(video_menu_data));
    memset(local, 0, sizeof(video_menu_data));

    // Load settings etc.
    const char* offon_opts[] = {"OFF","ON"};
    settings *setting = settings_get();

    // Create menu and its header
    component* menu = menu_create(11);
    menu_attach(menu, label_create(&font_large, "VIDEO"));
    menu_attach(menu, filler_create());

    // Resolution selector
    component *res_selector = textselector_create(&font_large, "RES:", resolution_toggled, local);
    menu_attach(menu, res_selector);

    // If custom resolution is set, add it as first selection
    resolution *res = find_resolution_by_settings(setting);
    if(!res) {
        char tmp_label[32];
        sprintf(tmp_label, "%ux%u", setting->video.screen_w, setting->video.screen_h);
        textselector_add_option(res_selector, tmp_label);
        local->custom_resolution.x = setting->video.screen_w;
        local->custom_resolution.y = setting->video.screen_h;
        local->is_custom_resolution = 1;
    }

    // Add standard resolutions
    for(int i = 0; i < sizeof(_resolutions)/sizeof(resolution); i++) {
        textselector_add_option(res_selector, _resolutions[i].name);
        if(!local->is_custom_resolution&& _resolutions[i].w == res->w && _resolutions[i].h == res->h) {
            textselector_set_pos(res_selector, i);
        }
    }

    // vsync and fullscreen
    menu_attach(menu, textselector_create_bind_opts(&font_large, "VSYNC", NULL, NULL, &setting->video.vsync, offon_opts, 2));
    menu_attach(menu, textselector_create_bind_opts(&font_large, "FULLSCREEN", NULL, NULL, &setting->video.fullscreen, offon_opts, 2));

    // Scaler selection
    component *scaler = textselector_create(&font_large, "SCALER:", scaler_toggled, local);
    component *factor = textselector_create(&font_large, "SCALING FACTOR:", scaling_factor_toggled, local);
    menu_attach(menu, scaler);
    menu_attach(menu, factor);
    textselector_add_option(scaler, "NEAREST");
    textselector_add_option(factor, "1");
    local->scaler = scaler; // Save references to ease their use
    local->factor = factor;

    // Get scalers
    list mlist;
    list_create(&mlist);
    plugins_get_list_by_type(&mlist, "scaler");
    iterator it;
    list_iter_begin(&mlist, &it);
    base_plugin **plugin;
    int i = 1;
    int plugin_found = 0;
    while((plugin = iter_next(&it)) != NULL) {
        textselector_add_option(scaler, (*plugin)->get_name());
        if(strcmp((*plugin)->get_name(), setting->video.scaler) == 0) {
            textselector_set_pos(scaler, i);
            plugin_found = 1;
        }
        i++;
    }
    list_free(&mlist);
    component_disable(factor, !plugin_found);

    // Get scaling factors
    char tmp_buf[32];
    if(plugin_found) {
        // Get scaling factors
        int pindex = 0;
        int *plist;
        scaler_plugin scaler;
        scaler_init(&scaler);
        plugins_get_scaler(&scaler, setting->video.scaler);
        int plen = scaler_get_factors_list(&scaler, &plist);
        textselector_clear_options(factor);
        for(int i = 0; i < plen; i++) {
            sprintf(tmp_buf, "%d", plist[i]);
            textselector_add_option(factor, tmp_buf);
            if(plist[i] == setting->video.scale_factor ) {
                pindex = i;
            }
        }
        textselector_set_pos(factor, pindex);
    }

    // Done button
    menu_attach(menu, textbutton_create(&font_large, "DONE", COM_ENABLED, menu_video_done, NULL));

    // Userdata & free function for it
    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_video_free);
    return menu;
}
