#include <stdlib.h>
#include <SDL2/SDL.h>

#include "game/protos/object.h"
#include "game/scenes/mechlab.h"
#include "game/protos/scene.h"
#include "game/text/text.h"
#include "game/game_state.h"
#include "utils/log.h"
#include "resources/ids.h"

/*
#include "engine.h"
#include "utils/log.h"
#include "game/text/text.h"
#include "audio/music.h"
#include "video/video.h"
#include "game/settings.h"
#include "game/scene.h"
#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
*/


#define NPANELBUTTON(p) sizeof(p)/sizeof(object)

typedef struct mechlab_hand_t {
    object obj;
    int pressing;
    object *buttons;
    unsigned int nbuttons;
    int sel;
    int moving;
    int prev_sel;
    float move_interp;
} mechlab_hand;


typedef struct mechlab_local_t {
    object bg_obj[3];
    object panel_obj;

    object main_buttons[10];
    object yesno_buttons[2];
    object select_buttons[3];
    object training_buttons[4];
    object ailevel_buttons[4];
    object upgrade_buttons[11];

    mechlab_hand hand;

} mechlab_local;

void panelbutton_create(object *pb, unsigned int npb, scene *scene, unsigned int anim) {
    for(int i = 0;i < npb; i++) {
        sprite *button_spr = sprite_copy(animation_get_sprite(&bk_get_info(&scene->bk_data, anim)->ani, i));
        animation *button_ani = create_animation_from_single(button_spr, vec2i_create(0,0));
        object_create(&pb[i], pb->gs, button_spr->pos, vec2f_create(0,0));
        object_set_animation(&pb[i], button_ani);
        object_select_sprite(&pb[i], 0);
        object_set_repeat(&pb[i], 1);
        object_set_animation_owner(&pb[i], OWNER_OBJECT);
    }
}

void panelbutton_free(object *pb, unsigned int npb) {
    for(int i = 0;i < npb; i++) {
        object_free(&pb[i]);
    }
}

void mechlab_free(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);

    panelbutton_free(local->ailevel_buttons, NPANELBUTTON(local->ailevel_buttons));
    panelbutton_free(local->upgrade_buttons, NPANELBUTTON(local->upgrade_buttons));
    panelbutton_free(local->yesno_buttons, NPANELBUTTON(local->yesno_buttons));
    panelbutton_free(local->select_buttons, NPANELBUTTON(local->select_buttons));
    panelbutton_free(local->main_buttons, NPANELBUTTON(local->main_buttons));
    panelbutton_free(local->training_buttons, NPANELBUTTON(local->training_buttons));

    for(int i = 0; i < sizeof(local->bg_obj)/sizeof(object); i++) {
        object_free(&local->bg_obj[i]);
    }
    object_free(&local->panel_obj);
    object_free(&local->hand.obj);
    free(local);
}

void mechlab_tick(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);

    if(local->hand.moving) {
        vec2i buttonsize_prev = object_get_size(&local->hand.buttons[local->hand.prev_sel]);
        vec2i buttonpos_prev = object_get_pos(&local->hand.buttons[local->hand.prev_sel]);
        vec2i buttonsize = object_get_size(&local->hand.buttons[local->hand.sel]);
        vec2i buttonpos = object_get_pos(&local->hand.buttons[local->hand.sel]);
        buttonsize_prev.x /= 2;
        buttonsize_prev.y /= 2;
        buttonsize.x /= 2;
        buttonsize.y /= 2;
        vec2f pos_prev = vec2i_to_f(vec2i_add(buttonpos_prev, buttonsize_prev));
        vec2f pos = vec2i_to_f(vec2i_add(buttonpos, buttonsize));
        vec2f target = vec2f_sub(pos, pos_prev);
        target.x *= local->hand.move_interp;
        target.y *= local->hand.move_interp;
        object_set_pos(&local->hand.obj, vec2f_to_i(vec2f_add(pos_prev, target)));
        local->hand.move_interp += 0.05f;

        if(local->hand.move_interp >= 1.0f) {
            local->hand.moving = 0;
            local->hand.move_interp = 0;
            local->hand.prev_sel = local->hand.sel;
        }
    } else if(local->hand.pressing)  {
        object_tick(&local->hand.obj);
    }
}

void mechlab_hand_sel_button(mechlab_local *local) {
    vec2i buttonpos = object_get_pos(&local->hand.buttons[local->hand.sel]);
    DEBUG("HAND POSITION %d %d", buttonpos.x, buttonpos.y);
    if(local->hand.sel == local->hand.prev_sel) {
        vec2i buttonsize = object_get_size(&local->hand.buttons[local->hand.sel]);
        buttonsize.x /= 2;
        buttonsize.y /= 2;
        object_set_pos(&local->hand.obj, vec2i_add(buttonpos, buttonsize));
    } else {
        local->hand.moving = 1;
    }

}

int mechlab_event(scene *scene, SDL_Event *event) {
    mechlab_local *local = scene_get_userdata(scene);
    int hand_moved = 0;

    if(event->type == SDL_KEYDOWN) {
        switch(event->key.keysym.sym) {
            case SDLK_ESCAPE:
                game_state_set_next(scene->gs, SCENE_MENU);
                break;

            case SDLK_RETURN:
                if(!(local->hand.moving || local->hand.pressing)) {
                    local->hand.pressing = 1;
                }
                break;

            // TODO selection order
            case SDLK_LEFT:
                if(!(local->hand.moving || local->hand.pressing)) {
                    local->hand.sel--;
                    local->hand.sel = (local->hand.sel < 0 ? local->hand.nbuttons-1 : local->hand.sel);
                    hand_moved = 1;
                }

            break;

            case SDLK_RIGHT:
                if(!(local->hand.moving || local->hand.pressing)) {
                    local->hand.sel++;
                    local->hand.sel = (local->hand.sel == local->hand.nbuttons ? 0 : local->hand.sel);
                    hand_moved = 1;
                }
            break;

            case SDLK_UP:
            break;

            case SDLK_DOWN:
            break;

        }

        if(hand_moved) {
            mechlab_hand_sel_button(local);
        }
        return 1;
    }
    return 0;
}

void mechlab_render(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);

    for(int i = 0; i < sizeof(local->bg_obj)/sizeof(object); i++) {
        object_render(&local->bg_obj[i]);
    }
    object_render(&local->panel_obj);
    object_render(&local->hand.obj);
}

void mechlab_hand_finished(object *hand_obj) {
    mechlab_local *local = object_get_userdata(hand_obj);
    local->hand.pressing = 0;
    player_reset(&local->hand.obj);
    object_tick(&local->hand.obj);
}

// Init mechlab
int mechlab_create(scene *scene) {
    // Alloc
    mechlab_local *local = malloc(sizeof(mechlab_local));
    animation *bg_ani[3];
    animation *panel_ani;

    // Init the background
    for(int i = 0; i < sizeof(bg_ani)/sizeof(animation*); i++) {
        sprite *spr = sprite_copy(animation_get_sprite(&bk_get_info(&scene->bk_data, 14)->ani, i));
        bg_ani[i] = create_animation_from_single(spr, spr->pos);
        object_create(&local->bg_obj[i], scene->gs, vec2i_create(0,0), vec2f_create(0,0));
        object_set_animation(&local->bg_obj[i], bg_ani[i]);
        object_select_sprite(&local->bg_obj[i], 0);
        object_set_repeat(&local->bg_obj[i], 1);
        object_set_animation_owner(&local->bg_obj[i], OWNER_OBJECT);
    }

    // Init the panel
    sprite *panel_spr = sprite_copy(animation_get_sprite(&bk_get_info(&scene->bk_data, 1)->ani, 2));
    panel_ani = create_animation_from_single(panel_spr, panel_spr->pos);
    object_create(&local->panel_obj, scene->gs, vec2i_create(0,0), vec2f_create(0,0));
    object_set_animation(&local->panel_obj, panel_ani);
    object_select_sprite(&local->panel_obj, 0);
    object_set_repeat(&local->panel_obj, 1);
    object_set_animation_owner(&local->panel_obj, OWNER_OBJECT);

    panelbutton_create(local->ailevel_buttons, NPANELBUTTON(local->ailevel_buttons), scene, 2);
    panelbutton_create(local->upgrade_buttons, NPANELBUTTON(local->upgrade_buttons), scene, 3);
    panelbutton_create(local->yesno_buttons, NPANELBUTTON(local->yesno_buttons), scene, 6);
    panelbutton_create(local->select_buttons, NPANELBUTTON(local->select_buttons), scene, 7);
    panelbutton_create(local->main_buttons, NPANELBUTTON(local->main_buttons), scene, 8);
    panelbutton_create(local->training_buttons, NPANELBUTTON(local->training_buttons), scene, 9);

    // Init hand
    local->hand.pressing = 0;
    local->hand.buttons = local->main_buttons;
    local->hand.nbuttons = NPANELBUTTON(local->main_buttons);
    local->hand.sel = 0;
    local->hand.prev_sel = 0;
    local->hand.moving = 0;
    local->hand.move_interp = 0;
    object_create(&local->hand.obj, scene->gs, vec2i_create(0,0), vec2f_create(0,0));
    object_set_userdata(&local->hand.obj, local);
    object_set_animation(&local->hand.obj, &bk_get_info(&scene->bk_data, 29)->ani);
    object_set_repeat(&local->hand.obj, 0);
    mechlab_hand_sel_button(local);
    object_set_finish_cb(&local->hand.obj, mechlab_hand_finished);
    object_tick(&local->hand.obj);

    // Set callbacks
    scene_set_userdata(scene, local);
    scene_set_event_cb(scene, mechlab_event);
    scene_set_render_cb(scene, mechlab_render);
    scene_set_free_cb(scene, mechlab_free);
    scene_set_tick_cb(scene, mechlab_tick);
    return 0;
}
