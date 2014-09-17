#include "game/scenes/mainmenu/menu_video_confirm.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"

typedef struct {
    time_t video_accept_timer;
    settings_video old_video_settings;
    int video_accept_secs;
    char video_accept_label[100];


    component video_confirm_header;
    component video_confirm_cancel;
    component video_confirm_ok;
} video_menu_confirm_data;


void video_confirm_cancel_clicked(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    settings_video *v = &settings_get()->video;
    *v = local->old_video_settings;
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync, v->scaler, v->scale_factor);
    mainmenu_prev_menu(c, userdata);
}

void menu_video_confirm_free(menu *menu) {
    video_menu_confirm_data *local = menu_get_userdata(menu);
    textbutton_free(&local->video_confirm_header);
    textbutton_free(&local->video_confirm_cancel);
    textbutton_free(&local->video_confirm_ok);
    free(local);
}

void menu_video_confirm_create(menu *menu) {
    video_menu_confirm_data *local = malloc(sizeof(video_menu_confirm_data));

    menu_set_userdata(local);
    menu_set_free_cb(menu_video_confirm_free);
}
