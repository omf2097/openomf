#include "game/scenes/mainmenu/menu_help.h"

#include "game/game_state.h"
#include "game/gui/gui.h"
#include "game/gui/text/text.h"
#include "game/protos/scene.h"
#include "game/utils/nat.h"
#include "game/utils/settings.h"
#include "resources/languages.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/video.h"

#define TEXT_SHADOW_GREEN 254

#define NUM_PAGES 13

typedef struct {
    scene *s;
    text_document *td;
    uint8_t page;
    surface background1;
    surface background2;
} help_menu_data;

void help_menu_update(help_menu_data *local) {
    text_document_free(&local->td);
    local->td = text_document_create();

    text_margin margin = {10, 0, 0, 0};

    str helpstr;
    str_from_c(&helpstr, lang_get(local->page));

    text_generate_document(local->td, &helpstr, FONT_BIG, 280, 170, TEXT_BRIGHT_GREEN, TEXT_SHADOW_GREEN,
                           TEXT_ALIGN_TOP, TEXT_ALIGN_LEFT, margin, 1, 0, 0, 0);

    str_free(&helpstr);
}

void menu_help_free(component *c) {
    help_menu_data *local = menu_get_userdata(c);
    text_document_free(&local->td);
    surface_free(&local->background1);
    surface_free(&local->background2);
    omf_free(local);
    menu_set_userdata(c, local);
}

void menu_help_render(component *c) {
    help_menu_data *local = menu_get_userdata(c);
    video_draw_remap(&local->background1, 15, 15, 4, 1, 0);
    video_draw(&local->background2, 15, 15);

    text_document_draw(local->td, 15, 20);
}

static int menu_help_event(component *c, SDL_Event *event) {
    help_menu_data *local = menu_get_userdata(c);
    SDL_Scancode sc = event->key.keysym.scancode;
    if(sc == SDL_SCANCODE_PAGEUP && local->page > 0) {
        local->page--;
        help_menu_update(local);
        return 1;
    } else if(sc == SDL_SCANCODE_PAGEDOWN && local->page < NUM_PAGES - 1) {
        local->page++;
        help_menu_update(local);
    }

    return 0;
}

static int menu_help_action(component *c, int action) {
    help_menu_data *local = menu_get_userdata(c);
    if((action == ACT_UP) && local->page > 0) {
        local->page--;
        help_menu_update(local);
        return 1;
    } else if((action == ACT_DOWN) && local->page < NUM_PAGES - 1) {
        local->page++;
        help_menu_update(local);
        return 1;
    } else if(action == ACT_ESC) {
        menu *m = sizer_get_obj(c);
        m->finished = 1;
    }

    return 0;
}

component *menu_help_create(scene *s) {
    help_menu_data *local = omf_calloc(1, sizeof(help_menu_data));
    local->s = s;
    local->page = 0;

    menu_transparent_bg_create(&local->background1, 290, 170);
    menu_background_create(&local->background2, 290, 170, MenuBackground);

    help_menu_update(local);

    component *menu = menu_create();

    menu_set_free_cb(menu, menu_help_free);
    component_set_render_cb(menu, menu_help_render);

    component_set_event_cb(menu, menu_help_event);
    component_set_action_cb(menu, menu_help_action);

    menu_set_userdata(menu, local);

    return menu;
}
