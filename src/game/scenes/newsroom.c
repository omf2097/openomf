#include "game/scenes/newsroom.h"
#include "game/menu/menu_background.h"
#include "game/text/languages.h"
#include "game/text/text.h"
#include "game/settings.h"
#include "audio/music.h"
#include "resources/ids.h"
#include "resources/pilots.h"
#include "utils/random.h"
#include "utils/str.h"
#include "utils/log.h"
#include "video/surface.h"
#include "video/video.h"

// newsroom text starts at 87 in english.dat
// there are 24*2 texts in total
#define NEWSROOM_TEXT 87

typedef struct newsroom_local_t {
    int news_id;
    int screen;
    surface news_bg;
    str news_str;
    str pilot1, pilot2;
    str har1, har2;
    int sex1, sex2;
    int won;
} newsroom_local;

char *object_pronoun(int sex) {
  if (sex == PILOT_SEX_MALE) {
    return "Him";
  }
  return "Her";
}

char *subject_pronoun(int sex) {
  if (sex == PILOT_SEX_MALE) {
    return "He";
  }
  return "She";
}

void newsroom_fixup_str(newsroom_local *local) {

    /*
     * Substitution table

       1= Player1 - Crystal
       2= Player2 - Steffan
       3= HAR1 - jaguar
       4= HAR2 - shadow
       5= Arena - power plant
       6= His/Her P1 - Her
       7= Him/Her P1 - Her
       8= He/She P1 - She
       10= Him/Her P2 - Him
       11= He/She P2 - He
    */

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
                    str_append_c(&local->news_str, object_pronoun(local->sex2));
                    pos++;
                } else if(nn == '1') {
                    // ~11
                    str_append_c(&local->news_str, subject_pronoun(local->sex2));
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
                str_append_c(&local->news_str, object_pronoun(local->sex1));
                break;
            case '8':
                str_append_c(&local->news_str, subject_pronoun(local->sex1));
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
    surface_free(&local->news_bg);
    str_free(&local->news_str);
    str_free(&local->pilot1);
    str_free(&local->pilot2);
    str_free(&local->har1);
    str_free(&local->har2);
    free(local);
}

void newsroom_tick(scene *scene) {


}

void newsroom_overlay_render(scene *scene) {
    newsroom_local *local = scene_get_userdata(scene);

    if(str_size(&local->news_str) > 0) {
        video_render_sprite(&local->news_bg, 20, 140, BLEND_ALPHA, 0);
        font_render_wrapped(&font_small, str_c(&local->news_str), 30, 150, 250, COLOR_YELLOW);
    }
}


int newsroom_event(scene *scene, SDL_Event *event) {
    newsroom_local *local = scene_get_userdata(scene);

    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *p1=NULL, *i;
    controller_event(player1->ctrl, event, &p1);
    i = p1;
    if (i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if (
                        i->event_data.action == ACT_ESC ||
                        i->event_data.action == ACT_KICK ||
                        i->event_data.action == ACT_PUNCH) {
                    local->screen++;
                    newsroom_fixup_str(local);
                    if(local->screen >= 2) {
                        if (local->won) {
                            // pick a new player
                            game_player *p1 = game_state_get_player(scene->gs, 0);
                            game_player *p2 = game_state_get_player(scene->gs, 1);
                            DEBUG("wins are %d", p1->sp_wins);
                            if (p1->sp_wins == (4094 ^ (2 << p1->pilot_id)))  {
                                // won the game
                                game_state_set_next(scene->gs, SCENE_END);
                            } else {
                                if (p1->sp_wins == (2046 ^ (2 << p1->pilot_id))) {
                                    // everyone but kriessack
                                    p2->pilot_id = 10;
                                    p2->har_id = HAR_NOVA;
                                } else {
                                    // pick an opponent we have not yet beaten
                                    while(1) {
                                        int i = rand_int(10);
                                        if ((2 << i) & p1->sp_wins || i == p1->pilot_id) {
                                            continue;
                                        }
                                        p2->pilot_id = i;
                                        p2->har_id = HAR_JAGUAR + rand_int(10);
                                        break;
                                    }
                                }
                                pilot p;
                                pilot_get_info(&p, p2->pilot_id);
                                p2->colors[0] = p.colors[0];
                                p2->colors[1] = p.colors[1];
                                p2->colors[2] = p.colors[2];

                                // make a new AI controller
                                controller *ctrl = malloc(sizeof(controller));
                                controller_init(ctrl);
                                ai_controller_create(ctrl, settings_get()->gameplay.difficulty);
                                game_player_set_ctrl(p2, ctrl);
                                // TODO set player's colors
                                game_state_set_next(scene->gs, SCENE_VS);
                            }
                        } else {
                            game_state_set_next(scene->gs, SCENE_MENU);
                        }
                    }
                }
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
    return 0;
}

int pilot_sex(int pilot_id) {
  switch (pilot_id) {
    case 0:
      // Crystal
    case 7:
      // Angel
    case 8:
      // Cossette
      return PILOT_SEX_FEMALE;
    default:
      // everyone else is male
      return PILOT_SEX_MALE;
  }
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

    if(!music_playing()) {
        char *filename = get_path_by_id(PSM_MENU);
        music_play(filename);
        free(filename);
    }

    game_player *p1 = game_state_get_player(scene->gs, 0);
    game_player *p2 = game_state_get_player(scene->gs, 1);

    local->won = 1;
    if (p2->sp_wins > 0) {
      // AI won, player lost
      local->won = 0;
    }

    // XXX TODO get the real sex of pilot
    // XXX TODO strip spaces from the end of the pilots name
    // XXX TODO set winner/loser names properly
    newsroom_set_names(local, lang_get(20+p1->pilot_id),
                              lang_get(20+p2->pilot_id),
                              get_id_name(p1->har_id),
                              get_id_name(p2->har_id),
                              pilot_sex(p1->pilot_id), pilot_sex(p2->pilot_id));
    newsroom_fixup_str(local);

    // Set callbacks
    scene_set_userdata(scene, local);
    scene_set_event_cb(scene, newsroom_event);
    scene_set_render_overlay_cb(scene, newsroom_overlay_render);
    scene_set_free_cb(scene, newsroom_free);
    scene_set_tick_cb(scene, newsroom_tick);

    // Pick renderer
    video_select_renderer(VIDEO_RENDERER_HW);
    
    return 0;
}
