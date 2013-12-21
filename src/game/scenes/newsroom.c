#include "game/scenes/newsroom.h"
#include "game/menu/menu_background.h"
#include "game/text/languages.h"
#include "game/text/text.h"
#include "resources/ids.h"
#include "utils/random.h"
#include "utils/str.h"
#include "utils/log.h"
#include "video/texture.h"
#include "video/video.h"

// newsroom text starts at 87 in english.dat
// there are 24*2 texts in total
#define NEWSROOM_TEXT 87

typedef struct newsroom_local_t {
    int news_id;
    int screen;
    texture news_bg;
    texture ss;
    str news_str;
    str pilot1, pilot2;
    str har1, har2;
    int sex1, sex2;
} newsroom_local;


void newsroom_fixup_str(newsroom_local *local) {

    // replace ~1 with winners name
    // ~2 with losers name
    // ~3 with winners har name
    // ~4 with losers har name
    // ~5 with Stadium
    // ~6 with The
    // ~7 with Her
    // ~8 with She
    // ~10 with Her
    // ~11 with She

    const char *text = NULL;

    if(local->screen == 0) {
        text = lang_get(NEWSROOM_TEXT+local->news_id);
    } else {
        text = lang_get(NEWSROOM_TEXT+local->news_id+1);
    }

    str textstr;
    size_t prevpos=0, pos = 0;

    str_create_from_cstr(&textstr, text);
    str_free(&local->news_str);
    while(str_next_of(&textstr, '~', &pos)) {
        str tmp;
        str_create(&tmp);
        str_substr(&tmp, &textstr, prevpos, pos);
        str_append(&local->news_str, &tmp);
        str_free(&tmp);

        // replace ~n tokens
        char n = str_at(&textstr, pos+1);
        char nn = str_at(&textstr, pos+2);
        switch(n) {
            case '1':
                if(nn == '0') {
                    // ~10
                    str_append_c(&local->news_str, "Her");
                    pos++;
                } else if(nn == '1') {
                    // ~11
                    str_append_c(&local->news_str, "She");
                    pos++;
                } else {
                    // ~1
                    str_append(&local->news_str, &local->pilot1);
                }
                break;
            case '2':
                str_append(&local->news_str, &local->pilot2);
                break;
            case '3':
                str_append(&local->news_str, &local->har1);
                break;
            case '4':
                str_append(&local->news_str, &local->har2);
                break;
            case '5':
                str_append_c(&local->news_str, "Stadium");
                break;
            case '6':
                str_append_c(&local->news_str, "The");
                break;
            case '7':
                str_append_c(&local->news_str, "Her");
                break;
            case '8':
                str_append_c(&local->news_str, "She");
                break;
            case '9':
                str_append_c(&local->news_str, "WTF");
                break;
        }
        pos+=2;
        prevpos = pos;
    }
    str tmp;
    str_create(&tmp);
    str_substr(&tmp, &textstr, pos, str_size(&textstr));
    str_append(&local->news_str, &tmp);
    str_free(&tmp);
    str_free(&textstr);
}

void newsroom_set_names(newsroom_local *local,
                        const char *pilot1, const char *pilot2,
                        const char *har1, const char *har2,
                        int sex1, int sex2) {

    str_create_from_cstr(&local->pilot1, pilot1);
    str_create_from_cstr(&local->pilot2, pilot2);
    str_create_from_cstr(&local->har1, har1);
    str_create_from_cstr(&local->har2, har2);
    local->sex1 = sex1;
    local->sex2 = sex2;
}


// newsroom callbacks
void newsroom_free(scene *scene) {
    newsroom_local *local = scene_get_userdata(scene);
    texture_free(&local->news_bg);
    str_free(&local->news_str);
    str_free(&local->pilot1);
    str_free(&local->pilot2);
    str_free(&local->har1);
    str_free(&local->har2);
    texture_free(&local->ss);
    free(local);
}

void newsroom_tick(scene *scene) {


}

void newsroom_overlay_render(scene *scene) {
    newsroom_local *local = scene_get_userdata(scene);

    if(str_size(&local->news_str) > 0) {
        video_render_sprite_flip(&local->news_bg, 20, 140, BLEND_ALPHA_FULL, FLIP_NONE);
        font_render_wrapped(&font_small, str_c(&local->news_str), 30, 150, 250, COLOR_YELLOW);
    }
}


int newsroom_event(scene *scene, SDL_Event *event) {
    newsroom_local *local = scene_get_userdata(scene);

    if(event->type == SDL_KEYDOWN) {
        switch(event->key.keysym.sym) {
            case SDLK_ESCAPE:
            case SDLK_RETURN:
                local->screen++;
                newsroom_fixup_str(local);
                if(local->screen >= 2) {
                    game_state_set_next(scene->gs, SCENE_MENU);
                }
                break;
        }

        return 1;
    }
    return 0;
}

int newsroom_create(scene *scene) {
    newsroom_local *local = malloc(sizeof(newsroom_local));

    local->news_id = rand_int(24)*2;
    local->screen = 0;
    menu_background_create(&local->news_bg, 280, 50);
    str_create(&local->news_str);
    str_create(&local->pilot1);
    str_create(&local->pilot2);
    str_create(&local->har1);
    str_create(&local->har2);
    texture_create(&local->ss);

    game_player *p1 = game_state_get_player(scene->gs, 0);
    game_player *p2 = game_state_get_player(scene->gs, 1);

    // XXX TODO get the real sex of pilot
    // XXX TODO strip spaces from the end of the pilots name
    // XXX TODO set winner/loser names properly
    newsroom_set_names(local, lang_get(20+p1->pilot_id),
                              lang_get(20+p2->pilot_id),
                              get_id_name(p1->har_id),
                              get_id_name(p2->har_id),
                              PILOT_SEX_MALE, PILOT_SEX_MALE);
    newsroom_fixup_str(local);

    // Set callbacks
    scene_set_userdata(scene, local);
    scene_set_event_cb(scene, newsroom_event);
    scene_set_render_overlay_cb(scene, newsroom_overlay_render);
    scene_set_free_cb(scene, newsroom_free);
    scene_set_tick_cb(scene, newsroom_tick);
    return 0;
}
