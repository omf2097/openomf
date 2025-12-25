#include <stdio.h>

#include "game/scenes/mainmenu/menu_video.h"
#include "game/scenes/mainmenu/menu_video_confirm.h"

#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "utils/allocator.h"
#include "utils/c_array_util.h"
#include "utils/c_string_util.h"
#include "video/video.h"

struct resolution_t {
    int w;
    int h;
    const char *name;
} _resolutions[] = {
    {320,  200,  "320x200"  },
    {320,  240,  "320x240"  },
    {640,  400,  "640x400"  },
    {640,  480,  "640x480"  },
    {800,  480,  "800x480"  },
    {800,  600,  "800x600"  },
    {960,  600,  "960x600"  },
    {1024, 768,  "1024x768" },
    {1280, 720,  "1280x720" },
    {1280, 800,  "1280x800" },
    {1280, 960,  "1280x960" },
    {1280, 1024, "1280x1024"},
    {1366, 768,  "1366x768" },
    {1440, 900,  "1440x900" },
    {1600, 900,  "1600x900" },
    {1600, 1000, "1600x1000"},
    {1600, 1200, "1600x1200"},
    {1680, 1050, "1680x1050"},
    {1920, 1080, "1920x1080"},
    {1920, 1200, "1920x1200"},
    {1920, 1440, "1920x1440"},
    {2560, 1440, "2560x1440"},
    {2560, 1600, "2560x1600"},
    {3840, 2160, "3840x2160"},
    {4096, 2160, "4096x2160"},
    {4096, 3072, "4096x3072"},
    {7680, 4320, "7680x4320"},
    {8192, 4320, "8192x4320"},
};

struct framerate_t {
    int rate;
    const char *name;
    // TODO: Add 160, 240, 360 when we have a more precise delay method.
} _framerates[] = {
    {0,   "None"   },
    {30,  "30 fps" },
    {60,  "60 fps" },
    {75,  "75 fps" },
    {90,  "90 fps" },
    {120, "120 fps"},
    {144, "144 fps"},
};

struct framebuffer_scale_t {
    int factor;
    const char *name;
} _framebuffer_scaling_factors[] = {
    {1, "1x"},
    {2, "2x"},
    {4, "4x"},
    {8, "8x"},
};

typedef struct resolution_t resolution;
typedef struct framerate_t framerate;
typedef struct framebuffer_scale_t framebuffer_scale;

typedef struct {
    vec2i custom_resolution;
    bool is_custom_resolution;
    int custom_framerate_limit;
    bool is_custom_framerate_limit;
    settings_video old_video_settings;
} video_menu_data;

resolution *find_resolution_by_settings(settings *s) {
    int w = s->video.screen_w;
    int h = s->video.screen_h;

    for(unsigned i = 0; i < N_ELEMENTS(_resolutions); i++) {
        if(w == _resolutions[i].w && h == _resolutions[i].h) {
            return &_resolutions[i];
        }
    }
    return NULL;
}

framerate *find_framerate_by_settings(settings *s) {
    int rate = s->video.framerate_limit;
    for(unsigned i = 0; i < N_ELEMENTS(_framerates); i++) {
        if(rate == _framerates[i].rate) {
            return &_framerates[i];
        }
    }
    return NULL;
}

framebuffer_scale *find_framebuffer_scale_by_settings(settings *s) {
    int fb_scale = s->video.fb_scale;
    for(unsigned i = 0; i < N_ELEMENTS(_framebuffer_scaling_factors); i++) {
        if(fb_scale == _framebuffer_scaling_factors[i].factor) {
            return &_framebuffer_scaling_factors[i];
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
            v->screen_w = _resolutions[pos - 1].w;
            v->screen_h = _resolutions[pos - 1].h;
        }
    } else {
        v->screen_w = _resolutions[pos].w;
        v->screen_h = _resolutions[pos].h;
    }
}

void framerate_toggled(component *c, void *userdata, int pos) {
    video_menu_data *local = userdata;
    settings_video *v = &settings_get()->video;
    if(local->is_custom_framerate_limit) {
        if(pos == 0) {
            v->framerate_limit = local->custom_framerate_limit;
        } else {
            v->framerate_limit = _framerates[pos - 1].rate;
        }
    } else {
        v->framerate_limit = _framerates[pos].rate;
    }
}

void fb_scaling_toggled(component *c, void *userdata, int pos) {
    settings_video *v = &settings_get()->video;
    v->fb_scale = _framebuffer_scaling_factors[pos].factor;
}

void renderer_toggled(component *c, void *userdata, int pos) {
    settings_video *v = &settings_get()->video;
    const char *renderer;
    video_get_renderer_info(pos, &renderer, NULL);
    strncpy_or_abort(v->renderer, renderer, sizeof(v->renderer));
}

void menu_video_done(component *c, void *u) {
    scene *s = u;
    video_menu_data *local = menu_get_userdata(c->parent);
    settings_video *v = &settings_get()->video;

    bool render_plugin_changed = strcmp(v->renderer, local->old_video_settings.renderer) != 0;
    if(render_plugin_changed) {
        video_close();
        video_init(v->renderer, v->screen_w, v->screen_h, v->fullscreen, v->vsync, v->aspect, v->framerate_limit,
                   v->fb_scale);
        menu_set_submenu(c->parent, menu_video_confirm_create(s, &local->old_video_settings));
    } else if(local->old_video_settings.screen_w != v->screen_w || local->old_video_settings.screen_h != v->screen_h ||
              local->old_video_settings.fullscreen != v->fullscreen || local->old_video_settings.vsync != v->vsync ||
              local->old_video_settings.aspect != v->aspect ||
              local->old_video_settings.framerate_limit != v->framerate_limit ||
              local->old_video_settings.fb_scale != v->fb_scale) {
        video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync, v->aspect, v->framerate_limit, v->fb_scale);

        menu_set_submenu(c->parent, menu_video_confirm_create(s, &local->old_video_settings));
    } else {
        menu *m = sizer_get_obj(c->parent);
        m->finished = 1;
    }
}

void menu_video_free(component *c) {
    video_menu_data *local = menu_get_userdata(c);
    omf_free(local);
    menu_set_userdata(c, local);
}

void menu_video_submenu_done(component *c, component *submenu) {
    menu *m = sizer_get_obj(c);
    m->finished = 1;
}

component *menu_video_create(scene *s) {
    // Menu userdata
    video_menu_data *local = omf_calloc(1, sizeof(video_menu_data));
    local->old_video_settings = settings_get()->video;

    // Load settings etc.
    const char *offon_opts[] = {"OFF", "ON"};
    const char *aspect_opts[] = {"4:3", "NATIVE"};
    settings *setting = settings_get();

    // Create menu and its header
    component *menu = menu_create();
    menu_attach(menu, label_create_title("VIDEO"));
    menu_attach(menu, filler_create());

    // Renderer selector
    component *renderer_selector =
        textselector_create("API:", "Choose the video renderer API to use", renderer_toggled, local);
    menu_attach(menu, renderer_selector);

    // Add available renderers and make sure correct one is selected by default.
    const char *r_name;
    for(int r = 0; r < video_get_renderer_count(); r++) {
        video_get_renderer_info(r, &r_name, NULL);
        if(strcmp(r_name, "NULL") == 0) {
            continue;
        }
        textselector_add_option(renderer_selector, r_name);
        if(strcmp(r_name, setting->video.renderer) == 0) {
            textselector_set_pos(renderer_selector, r);
        }
    }

    // Resolution selector
    component *res_selector =
        textselector_create("RES:", "Choose the game resolution to use.", resolution_toggled, local);
    menu_attach(menu, res_selector);

    // If custom resolution is set, add it as first selection
    resolution *res = find_resolution_by_settings(setting);
    if(!res) {
        char tmp_label[32];
        snprintf(tmp_label, 32, "%dx%d", setting->video.screen_w, setting->video.screen_h);
        textselector_add_option(res_selector, tmp_label);
        local->custom_resolution.x = setting->video.screen_w;
        local->custom_resolution.y = setting->video.screen_h;
        local->is_custom_resolution = true;
    }

    // Add standard resolutions
    for(unsigned i = 0; i < N_ELEMENTS(_resolutions); i++) {
        textselector_add_option(res_selector, _resolutions[i].name);
        if(!local->is_custom_resolution && _resolutions[i].w == res->w && _resolutions[i].h == res->h) {
            textselector_set_pos(res_selector, i);
        }
    }

    // Framerate limiter selector
    component *framerate_limit_selector = textselector_create(
        "FPS:", "Limit framerate to a given value. This is useful for e.g. power saving.", framerate_toggled, local);
    menu_attach(menu, framerate_limit_selector);

    // If custom framerate is set, add it as first selection
    framerate *limit = find_framerate_by_settings(setting);
    if(!limit) {
        char tmp_label[32];
        snprintf(tmp_label, 32, "%d Hz", setting->video.framerate_limit);
        textselector_add_option(framerate_limit_selector, tmp_label);
        local->custom_framerate_limit = setting->video.framerate_limit;
        local->is_custom_framerate_limit = true;
    }

    // Add standard framerates
    for(unsigned i = 0; i < N_ELEMENTS(_framerates); i++) {
        textselector_add_option(framerate_limit_selector, _framerates[i].name);
        if(!local->is_custom_framerate_limit && limit != NULL && _framerates[i].rate == limit->rate) {
            textselector_set_pos(framerate_limit_selector, i);
        }
    }

    // Framerate limiter selector
    component *fb_scaling = textselector_create("FB SCALE:",
                                                "Set internal framebuffer scaling factor. You need to bump this if you "
                                                "are using higher resolution sprites from mods.",
                                                fb_scaling_toggled, local);
    menu_attach(menu, fb_scaling);

    // Add standard framebuffer scaling factors
    framebuffer_scale *scale = find_framebuffer_scale_by_settings(setting);
    for(unsigned i = 0; i < N_ELEMENTS(_framebuffer_scaling_factors); i++) {
        textselector_add_option(fb_scaling, _framebuffer_scaling_factors[i].name);
        if(scale != NULL && _framebuffer_scaling_factors[i].factor == scale->factor) {
            textselector_set_pos(fb_scaling, i);
        }
    }

    // vsync and fullscreen
    menu_attach(menu, textselector_create_bind_opts("VSYNC", "Toggle vertical sync on or off.", NULL, NULL,
                                                    &setting->video.vsync, offon_opts, 2));
    menu_attach(menu, textselector_create_bind_opts("ASPECT", "Video aspect ratio. Original game is 4:3.", NULL, NULL,
                                                    &setting->video.aspect, aspect_opts, 2));
    menu_attach(menu, textselector_create_bind_opts("FULLSCREEN", "Run the game in a fullscreen window.", NULL, NULL,
                                                    &setting->video.fullscreen, offon_opts, 2));

    // Done button
    menu_attach(menu, button_create("DONE", "Return to the main menu.", false, false, menu_video_done, s));

    // Userdata & free function for it
    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_video_free);
    menu_set_submenu_done_cb(menu, menu_video_submenu_done);
    return menu;
}
