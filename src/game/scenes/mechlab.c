#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "engine.h"
#include "utils/log.h"
#include "game/text/text.h"
#include "audio/music.h"
#include "video/video.h"
#include "game/settings.h"
#include "game/scene.h"
#include "game/scenes/mechlab.h"
#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"

#define NPANELBUTTON(p) sizeof(p)/sizeof(panelbutton)

typedef struct panelbutton_t {
    texture tex;
    sd_sprite *spr;
} panelbutton;

texture bgtex[3];
texture paneltex;

sd_sprite *panelspr;
sd_sprite *bgspr[3];

int handpoking = 0;
animationplayer handplayer;

panelbutton main_buttons[10];
panelbutton yesno_buttons[2];
panelbutton select_buttons[3];
panelbutton training_buttons[4];
panelbutton ailevel_buttons[4];
panelbutton upgrade_buttons[11];

struct hand_t {
    panelbutton *buttons;
    unsigned int nbuttons;
    int sel;
} hand = {main_buttons, NPANELBUTTON(main_buttons), 0};

void sprtotex(scene *scene, texture *tex, sd_sprite *spr) {
    sd_rgba_image *rgba = sd_sprite_image_decode(spr->img, scene->bk->palettes[0], -1);
    texture_create(tex, rgba->data, rgba->w, rgba->h);
    sd_rgba_image_delete(rgba);
}
void panelbutton_create(panelbutton *pb, unsigned int npb, scene *scene, unsigned int anim) {
    for(int i = 0;i < npb; i++) {
        pb[i].spr = scene->bk->anims[anim]->animation->sprites[i];
        sprtotex(scene, &pb[i].tex, pb[i].spr);
    }
}
void panelbutton_free(panelbutton *pb, unsigned int npb) {
    for(int i = 0;i < npb; i++) {
        texture_free(&pb[i].tex);
    }
}

// Init mechlab
int mechlab_init(scene *scene) {
    for(int i = 0;i < 3; i++) {
        bgspr[i] = scene->bk->anims[14]->animation->sprites[i];
        sprtotex(scene, &bgtex[i], bgspr[i]);
    }

    panelspr = scene->bk->anims[1]->animation->sprites[2];
    sprtotex(scene, &paneltex, panelspr);

    panelbutton_create(ailevel_buttons, NPANELBUTTON(ailevel_buttons), scene, 2);
    panelbutton_create(upgrade_buttons, NPANELBUTTON(upgrade_buttons), scene, 3);
    panelbutton_create(yesno_buttons, NPANELBUTTON(yesno_buttons), scene, 6);
    panelbutton_create(select_buttons, NPANELBUTTON(select_buttons), scene, 7);
    panelbutton_create(main_buttons, NPANELBUTTON(main_buttons), scene, 8);
    panelbutton_create(training_buttons, NPANELBUTTON(training_buttons), scene, 9);

    return 0;
}

void mechlab_post_init(scene *scene) {
    animationplayer_create(&handplayer, 29, array_get(&scene->animations, 29));
    animationplayer_run(&handplayer);
}

void mechlab_deinit(scene *scene) {
    for(int i = 0;i < 3; i++) {
        texture_free(&bgtex[i]);
    }
    animationplayer_free(&handplayer);
    texture_free(&paneltex);

    panelbutton_free(ailevel_buttons, NPANELBUTTON(ailevel_buttons));
    panelbutton_free(upgrade_buttons, NPANELBUTTON(upgrade_buttons));
    panelbutton_free(yesno_buttons, NPANELBUTTON(yesno_buttons));
    panelbutton_free(select_buttons, NPANELBUTTON(select_buttons));
    panelbutton_free(main_buttons, NPANELBUTTON(main_buttons));
    panelbutton_free(training_buttons, NPANELBUTTON(training_buttons));
}

void mechlab_tick(scene *scene) {
    if(handpoking) {
        animationplayer_run(&handplayer);

        if(handplayer.finished) {
            animationplayer_reset(&handplayer);
            animationplayer_run(&handplayer);
            handpoking = 0;
        }
    }
}

int mechlab_event(scene *scene, SDL_Event *event) {
    if(event->type == SDL_KEYDOWN) {
        switch(event->key.keysym.sym) {
            case SDLK_ESCAPE:
                scene->next_id = SCENE_MENU;
                break;

            case SDLK_RETURN:
                handpoking = 1;
                break;

            // TODO selection order
            case SDLK_LEFT:
                hand.sel--;
                hand.sel = (hand.sel < 0 ? hand.nbuttons-1 : hand.sel);
            break;

            case SDLK_RIGHT:
                hand.sel++;
                hand.sel = (hand.sel == hand.nbuttons ? 0 : hand.sel);
            break;

            case SDLK_UP:
            break;

            case SDLK_DOWN:
            break;
        }
        return 1;
    }
    return 0;
}

void mechlab_render(scene *scene) {
    for(int i = 0;i < 3; i++) {
        sd_sprite *spr = bgspr[i];
        video_render_sprite(&bgtex[i], spr->pos_x, spr->pos_y, BLEND_ALPHA_FULL);
    }
    video_render_sprite(&paneltex, panelspr->pos_x, panelspr->pos_y, BLEND_ALPHA_FULL);

    sd_sprite *selspr = hand.buttons[hand.sel].spr;
    handplayer.x = selspr->pos_x + selspr->img->w/2 ;
    handplayer.y = selspr->pos_y + selspr->img->h/2 ;
    animationplayer_render(&handplayer);
}

void mechlab_load(scene *scene) {
    scene->event = mechlab_event;
    scene->render = mechlab_render;
    scene->init = mechlab_init;
    scene->post_init = mechlab_post_init;
    scene->deinit = mechlab_deinit;
    scene->tick = mechlab_tick;
}

