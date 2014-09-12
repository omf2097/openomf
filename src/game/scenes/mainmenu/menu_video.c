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
    char custom_resolution_label[40];
    vec2i custom_resolution;
    int is_custom_resolution;
    char scaling_factor_labels[16][3];

    menu video_confirm_menu;
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
    menu_free(&local->video_confirm_menu);
    free(local);
}

void menu_video_create(menu *menu) {
    video_menu_data *local = malloc(sizeof(video_menu_data));

    textbutton_create(&local->video_header, &font_large, "VIDEO");
    resolution *res = find_resolution_by_settings(setting);
    if(res) {
        textselector_create(&local->resolution_toggle, &font_large, "RES:", _resolutions[0].name);
        local->is_custom_resolution = 0;
    } else {
        sprintf(local->custom_resolution_label, "%ux%u", setting->video.screen_w, setting->video.screen_h);
        textselector_create(&local->resolution_toggle, &font_large, "RES:", local->custom_resolution_label);
        local->custom_resolution.x = setting->video.screen_w;
        local->custom_resolution.y = setting->video.screen_h;
        local->is_custom_resolution = 1;
    }
    for(int i = local->is_custom_resolution ? 0 : 1;i < sizeof(_resolutions)/sizeof(resolution); ++i) {
        textselector_add_option(&local->resolution_toggle, _resolutions[i].name);
    }
    if(!local->is_custom_resolution) {
        textselector *t = local->resolution_toggle.obj;
        for(int i = 0;i < vector_size(&t->options);i++) {
            if(res->name == *(const char**)vector_get(&t->options, i)) {
                textselector_set_pos(&local->resolution_toggle, i);
                break;
            }
        }
    }

    textselector_create(&local->vsync_toggle, &font_large, "VSYNC:", "OFF");
    textselector_add_option(&local->vsync_toggle, "ON");
    textselector_create(&local->fullscreen_toggle, &font_large, "FULLSCREEN:", "OFF");
    textselector_add_option(&local->fullscreen_toggle, "ON");
    textselector_create(&local->scaler_toggle, &font_large, "SCALER:", "NEAREST");
    textselector_create(&local->scale_factor_toggle, &font_large, "SCALING FACTOR:", "1");

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
        textselector_add_option(&local->scaler_toggle, (*plugin)->get_name());
        if(strcmp((*plugin)->get_name(), setting->video.scaler) == 0) {
            textselector_set_pos(&local->scaler_toggle, i);
            plugin_found = 1;
        }
        i++;
    }
    list_free(&mlist);
    local->scale_factor_toggle.disabled = !plugin_found;

    if(plugin_found) {
        // Get scaling factors
        int pindex = 0;
        int *plist;
        scaler_plugin scaler;
        scaler_init(&scaler);
        plugins_get_scaler(&scaler, setting->video.scaler);
        int plen = scaler_get_factors_list(&scaler, &plist);
        textselector_clear_options(&local->scale_factor_toggle);
        for(int i = 0; i < plen; i++) {
            sprintf(local->scaling_factor_labels[i], "%d", plist[i]);
            textselector_add_option(&local->scale_factor_toggle, local->scaling_factor_labels[i]);
            if(plist[i] == setting->video.scale_factor ) {
                pindex = i;
            }
        }
        textselector_set_pos(&local->scale_factor_toggle, pindex);
    }

    textbutton_create(&local->video_done_button, &font_large, "DONE");
    menu_attach(&local->video_menu, &local->video_header, 22);
    menu_attach(&local->video_menu, &local->resolution_toggle, 11);
    menu_attach(&local->video_menu, &local->vsync_toggle, 11);
    menu_attach(&local->video_menu, &local->fullscreen_toggle, 11);
    menu_attach(&local->video_menu, &local->scaler_toggle, 11);
    menu_attach(&local->video_menu, &local->scale_factor_toggle, 11);
    menu_attach(&local->video_menu, &local->video_done_button, 11);
    local->video_header.disabled = 1;
    local->video_done_button.click = video_done_clicked;
    local->video_done_button.userdata = (void*)scene;
    menu_select(&local->video_menu, &local->resolution_toggle);

    local->resolution_toggle.toggle = resolution_toggled;
    local->resolution_toggle.userdata = (void*)scene;
    textselector_bindvar(&local->vsync_toggle, &setting->video.vsync);
    textselector_bindvar(&local->fullscreen_toggle, &setting->video.fullscreen);
    local->scaler_toggle.toggle = scaler_toggled;
    local->scaler_toggle.userdata = local;
    local->scale_factor_toggle.toggle = scaling_factor_toggled;
    local->scale_factor_toggle.userdata = local;

    menu_set_userdata(local);
    menu_set_free_cb(menu_video_free);
}
