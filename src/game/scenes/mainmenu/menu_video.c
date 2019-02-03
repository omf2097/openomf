#include <stdio.h>

#include "game/scenes/mainmenu/menu_video.h"
#include "game/scenes/mainmenu/menu_video_confirm.h"

#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "utils/compat.h"
#include "video/video.h"
#include "plugins/plugins.h"

struct resolution_t {
    int w;  int h;  const char *name;
} _resolutions[] = {
    {320,   200,    "320x200"},
    {640,   400,    "640x400"},
    {800,   480,    "800x480"},
    {800,   600,    "800x600"},
    {960,   600,    "960x600"},
    {1024,  768,    "1024x768"},
    {1280,  720,    "1280x720"},
    {1280,  800,    "1280x800"},
    {1280,  960,    "1280x960"},
    {1280,  1024,   "1280x1024"},
    {1366,  768,    "1366x768"},
    {1440,  900,    "1440x900"},
    {1600,  900,    "1600x900"},
    {1600,  1000,   "1600x1000"},
    {1600,  1200,   "1600x1200"},
    {1680,  1050,   "1680x1050"},
    {1920,  1080,   "1920x1080"},
    {1920,  1200,   "1920x1200"},
    {1920,  1440,   "1920x1440"},
    {2560,  1440,   "2560x1440"},
    {2560,  1600,   "2560x1600"},
    {3840,  2160,   "3840x2160"},
    {4096,  2160,   "4096x2160"},
    {4096,  3072,   "4096x3072"},
    {7680,  4320,   "7680x4320"},
    {8192,  4320,   "8192x4320"},
};

typedef struct resolution_t resolution;

typedef struct {
    vec2i custom_resolution;
    int is_custom_resolution;
    settings_video old_video_settings;
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

void scaler_toggled(component *c, void *userdata, int pos) {
    video_menu_data *local = userdata;
    settings_video *v = &settings_get()->video;

    // Set scaler
    free(v->scaler);
    v->scaler = strdup(textselector_get_current_text(c));

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
            snprintf(tmp_buf, 32, "%d", list[i]);
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
    scene *s = u;
    video_menu_data *local = menu_get_userdata(c->parent);
    settings_video *v = &settings_get()->video;
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync, v->scaler, v->scale_factor);

    if(local->old_video_settings.screen_w != v->screen_w ||
        local->old_video_settings.screen_h != v->screen_h ||
        local->old_video_settings.fullscreen != v->fullscreen ||
        local->old_video_settings.vsync != v->vsync) {

        menu_set_submenu(c->parent, menu_video_confirm_create(s, &local->old_video_settings));
    } else {
        menu *m = sizer_get_obj(c->parent);
        m->finished = 1;
    }
}

void menu_video_free(component *c) {
    video_menu_data *local = menu_get_userdata(c);
    free(local);
}

void menu_video_submenu_done(component *c, component *submenu) {
    menu *m = sizer_get_obj(c);
    m->finished = 1;
}

component* menu_video_create(scene *s) {
    // Menu userdata
    video_menu_data *local = malloc(sizeof(video_menu_data));
    memset(local, 0, sizeof(video_menu_data));
    local->old_video_settings = settings_get()->video;

    // Load settings etc.
    const char* offon_opts[] = {"OFF","ON"};
    settings *setting = settings_get();

    // Text config
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_BIG;
    tconf.halign = TEXT_CENTER;
    tconf.cforeground = color_create(0, 121, 0, 255);

    // Create menu and its header
    component* menu = menu_create(11);
    menu_attach(menu, label_create(&tconf, "VIDEO"));
    menu_attach(menu, filler_create());

    // Resolution selector
    component *res_selector = textselector_create(&tconf, "RES:", resolution_toggled, local);
    menu_attach(menu, res_selector);

    // If custom resolution is set, add it as first selection
    resolution *res = find_resolution_by_settings(setting);
    if(!res) {
        char tmp_label[32];
        snprintf(tmp_label, 32, "%ux%u", setting->video.screen_w, setting->video.screen_h);
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
    menu_attach(menu, textselector_create_bind_opts(&tconf, "VSYNC", NULL, NULL, &setting->video.vsync, offon_opts, 2));
    menu_attach(menu, textselector_create_bind_opts(&tconf, "FULLSCREEN", NULL, NULL, &setting->video.fullscreen, offon_opts, 2));

    // Scaler selection
    component *scaler = textselector_create(&tconf, "SCALER:", scaler_toggled, local);
    component *factor = textselector_create(&tconf, "SCALING FACTOR:", scaling_factor_toggled, local);
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
            snprintf(tmp_buf, 32, "%d", plist[i]);
            textselector_add_option(factor, tmp_buf);
            if(plist[i] == setting->video.scale_factor ) {
                pindex = i;
            }
        }
        textselector_set_pos(factor, pindex);
    }

    // Done button
    menu_attach(menu, textbutton_create(&tconf, "DONE", COM_ENABLED, menu_video_done, s));

    // Userdata & free function for it
    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_video_free);
    menu_set_submenu_done_cb(menu, menu_video_submenu_done);
    return menu;
}
