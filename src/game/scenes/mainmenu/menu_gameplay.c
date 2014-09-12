#include "game/scenes/mainmenu/menu_gameplay.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"

typedef struct {
    component gameplay_header;
    component speed_slider;
    component fightmode_toggle;
    component powerone_slider;
    component powertwo_slider;
    component hazards_toggle;
    component cpu_toggle;
    component round_toggle;
    component gameplay_done_button;
} gameplay_menu_data;

void menu_gameplay_free(menu *menu) {
    gameplay_menu_data *local = menu_get_userdata(menu);
    textbutton_free(&local->gameplay_header);
    textslider_free(&local->speed_slider);
    textselector_free(&local->fightmode_toggle);
    textslider_free(&local->powerone_slider);
    textslider_free(&local->powertwo_slider);
    textselector_free(&local->hazards_toggle);
    textselector_free(&local->cpu_toggle);
    textselector_free(&local->round_toggle);
    textbutton_free(&local->gameplay_done_button);
    free(local);
}

void menu_gameplay_create(menu *menu) {
    gameplay_menu_data *local = malloc(sizeof(gameplay_menu_data));

    textbutton_create(&local->gameplay_header, &font_large, "GAMEPLAY");
    textslider_create(&local->speed_slider, &font_large, "SPEED", 10, 0);
    textselector_create(&local->fightmode_toggle, &font_large, "FIGHT MODE", "NORMAL");
    textselector_add_option(&local->fightmode_toggle, "HYPER");
    textslider_create(&local->powerone_slider, &font_large, "POWER 1", 8, 0);
    textslider_create(&local->powertwo_slider, &font_large, "POWER 2", 8, 0);
    textselector_create(&local->hazards_toggle, &font_large, "HAZARDS", "OFF");
    textselector_add_option(&local->hazards_toggle, "ON");
    textselector_create(&local->cpu_toggle, &font_large, "CPU:", ai_difficulty_get_name(0));
    for(int i = 1; i < NUMBER_OF_AI_DIFFICULTY_TYPES; i++) {
        textselector_add_option(&local->cpu_toggle, ai_difficulty_get_name(i));
    }
    textselector_create(&local->round_toggle, &font_large, "", round_get_name(0));
    for(int i = 1; i < NUMBER_OF_ROUND_TYPES; i++) {
        textselector_add_option(&local->round_toggle, round_get_name(i));
    }
    textbutton_create(&local->gameplay_done_button, &font_large, "DONE");
    menu_attach(&local->gameplay_menu, &local->gameplay_header, 22);
    menu_attach(&local->gameplay_menu, &local->speed_slider, 11);
    menu_attach(&local->gameplay_menu, &local->fightmode_toggle, 11);
    menu_attach(&local->gameplay_menu, &local->powerone_slider, 11);
    menu_attach(&local->gameplay_menu, &local->powertwo_slider, 11);
    menu_attach(&local->gameplay_menu, &local->hazards_toggle, 11);
    menu_attach(&local->gameplay_menu, &local->cpu_toggle, 11);
    menu_attach(&local->gameplay_menu, &local->round_toggle, 11);
    menu_attach(&local->gameplay_menu, &local->gameplay_done_button, 11);

    // gameplay options
    textslider_bindvar(&local->speed_slider, &setting->gameplay.speed);
    textslider_bindvar(&local->powerone_slider, &setting->gameplay.power1);
    textslider_bindvar(&local->powertwo_slider, &setting->gameplay.power2);
    textselector_bindvar(&local->fightmode_toggle, &setting->gameplay.fight_mode);
    textselector_bindvar(&local->hazards_toggle, &setting->gameplay.hazards_on);
    textselector_bindvar(&local->cpu_toggle, &setting->gameplay.difficulty);
    textselector_bindvar(&local->round_toggle, &setting->gameplay.rounds);

    local->gameplay_header.disabled = 1;
    menu_select(&local->gameplay_menu, &local->speed_slider);

    local->speed_slider.userdata = (void*)scene;
    local->speed_slider.slide = menu_speed_slide;

    local->gameplay_done_button.click = mainmenu_prev_menu;
    local->gameplay_done_button.userdata = (void*)scene;
    
    menu_set_userdata(local);
    menu_set_free_cb(menu_gameplay_free);
}
