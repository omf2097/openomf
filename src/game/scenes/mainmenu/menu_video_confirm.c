#include <stdio.h>
#include <time.h>

#include "game/scenes/mainmenu/menu_video_confirm.h"

#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "video/video.h"
#include "utils/allocator.h"

typedef struct {
    time_t video_accept_timer;
    settings_video *old_video_settings;
    int video_accept_secs;
    component *timeout_label;
} video_menu_confirm_data;

void video_confirm_ok_clicked(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;

    video_menu_confirm_data *local = userdata;
    *local->old_video_settings = settings_get()->video;
}

void video_confirm_cancel_clicked(component *c, void *userdata) {
    video_menu_confirm_data *local = userdata;
    settings_video *v = &settings_get()->video;
    *v = *local->old_video_settings;
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync, v->scaler, v->scale_factor);

    // Finish the menu
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

void menu_video_confirm_free(component *c) {
    video_menu_confirm_data *local = menu_get_userdata(c);
    omf_free(local);
    menu_set_userdata(c, local);
}

void  menu_video_confirm_update(component *c) {
    video_menu_confirm_data *local = menu_get_userdata(c);
    char buf[32];
    snprintf(buf, 32, "CANCELLING IN %d", local->video_accept_secs);
    label_set_text(local->timeout_label, buf);
}

void menu_video_confirm_tick(component *c) {
    video_menu_confirm_data *local = menu_get_userdata(c);
    if(difftime(time(NULL), local->video_accept_timer) >= 1.0) {
        time(&local->video_accept_timer);
        local->video_accept_secs--;
        menu_video_confirm_update(c);
    }
    if(local->video_accept_secs == 0) {
        video_confirm_cancel_clicked(c, local);
    }
}

component* menu_video_confirm_create(scene *s, settings_video *old_settings) {
    video_menu_confirm_data *local = omf_calloc(1, sizeof(video_menu_confirm_data));
    local->video_accept_secs = 20;
    local->old_video_settings = old_settings;
    time(&local->video_accept_timer);

    // Text config
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_BIG;
    tconf.halign = TEXT_CENTER;
    tconf.cforeground = color_create(0, 121, 0, 255);

    component* menu = menu_create(11);
    menu_attach(menu, label_create(&tconf, "ACCEPT RESOLUTION?"));
    menu_attach(menu, filler_create());
    local->timeout_label = label_create(&tconf, "");
    menu_attach(menu, local->timeout_label);
    menu_attach(menu, filler_create());
    menu_attach(menu, textbutton_create(&tconf, "OK", COM_ENABLED, video_confirm_ok_clicked, local));
    menu_attach(menu, textbutton_create(&tconf, "CANCEL", COM_ENABLED, video_confirm_cancel_clicked, local));

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_video_confirm_free);
    menu_set_tick_cb(menu, menu_video_confirm_tick);

    menu_video_confirm_update(menu);
    return menu;
}
