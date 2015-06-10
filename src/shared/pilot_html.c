#include <stdio.h>
#include "pilot_html.h"

static const char *har_list[] = {
    "Jaguar",
    "Shadow",
    "Thorn",
    "Pyros",
    "Electra",
    "Katana",
    "Shredder",
    "Flail",
    "Gargoyle",
    "Chronos",
    "Nova"
};

static const char *difficulty_names[] = {
    "Aluminum",
    "Iron",
    "Steel",
    "Heavy Metal",
};

void print_pilot_player_info_html(FILE *f, sd_pilot *pilot) {
    if(pilot) {
        fprintf(f, "<tr><td>Name</td><td>%s</td></tr>", pilot->name);
        fprintf(f, "<tr><td>Wins:</td><td>%d</td></tr>", pilot->wins);
        fprintf(f, "<tr><td>Losses:</td><td>%d</td></tr>", pilot->losses);
        fprintf(f, "<tr><td>Rank:</td><td>%d</td></tr>", pilot->rank);
        if(pilot->har_id == 255)
            fprintf(f, "<tr><td>Har:</td><td>Random</td></tr>");
        else
            fprintf(f, "<tr><td>Har:</td><td>%s</td></tr>", har_list[pilot->har_id]);
        fprintf(f, "<tr><td>Arm Power:</td><td>%d</td></tr>", pilot->arm_power);
        fprintf(f, "<tr><td>Leg Power:</td><td>%d</td></tr>", pilot->leg_power);
        fprintf(f, "<tr><td>Arm Speed:</td><td>%d</td></tr>", pilot->arm_speed);
        fprintf(f, "<tr><td>Leg Speed:</td><td>%d</td></tr>", pilot->leg_speed);
        fprintf(f, "<tr><td>Armor:</td><td>%d</td></tr>", pilot->armor);
        fprintf(f, "<tr><td>Stun Res.:</td><td>%d</td></tr>", pilot->stun_resistance);
        fprintf(f, "<tr><td>Power:</td><td>%d</td></tr>", pilot->power);
        fprintf(f, "<tr><td>Agility:</td><td>%d</td></tr>", pilot->agility);
        fprintf(f, "<tr><td>Endurance:</td><td>%d</td></tr>", pilot->endurance);
        fprintf(f, "<tr><td>Offense:</td><td>%d</td></tr>", pilot->offense);
        fprintf(f, "<tr><td>Defense:</td><td>%d</td></tr>", pilot->defense);
        fprintf(f, "<tr><td>Money:</td><td>%d</td></tr>", pilot->money);
        fprintf(f, "<tr><td>Color:</td><td>%d,%d,%d</td></tr>",
            pilot->color_1,
            pilot->color_2,
            pilot->color_3);
    }
}

void print_pilot_info_html(FILE *f, sd_pilot *pilot) {
    if(pilot != NULL) {
        fprintf(f, "<div class=\"iblock\">");
        fprintf(f, "<h4>Pilot details</h4>");
        fprintf(f, "<table>");
        print_pilot_player_info_html(f, pilot);
        fprintf(f, "</table>");
        fprintf(f, "</div>");

        fprintf(f, "<div class=\"iblock\">");
        fprintf(f, "<h4>Tournament extra</h4>");
        fprintf(f, "<table>");
        fprintf(f, "<tr><td>TRN Name</td><td>%s</td></tr>", pilot->trn_name);
        fprintf(f, "<tr><td>TRN Desc</td><td>%s</td></tr>", pilot->trn_desc);
        fprintf(f, "<tr><td>TRN Image</td><td>%s</td></tr>", pilot->trn_image);
        fprintf(f, "<tr><td>Pilot ID</td><td>%d</td></tr>", pilot->pilot_id);
        fprintf(f, "<tr><td>Unknown K</td><td>%d</td></tr>", pilot->unknown_k);
        fprintf(f, "<tr><td>Force arena</td><td>%d</td></tr>", pilot->force_arena);
        fprintf(f, "<tr><td>Difficulty</td><td>%s</td></tr>", difficulty_names[pilot->difficulty]);
        fprintf(f, "<tr><td>Movement</td><td>%d</td></tr>", pilot->movement);
        fprintf(f, "</table>");
        fprintf(f, "</div>");

        fprintf(f, "<div class=\"iblock\">");
        fprintf(f, "<h4>Enhancements</h4>");
        fprintf(f, "<table>");
        for(int i = 0; i < 11; i++) {
            fprintf(f, "<tr><td>%s</td><td>%x</td></tr>", har_list[i], pilot->enhancements[i]);
        }
        fprintf(f, "</table>");
        fprintf(f, "</div>");

        fprintf(f, "<div class=\"iblock\">");
        fprintf(f, "<h4>Requirements</h4>");
        fprintf(f, "<table>");
        fprintf(f, "<tr><td>Secret</td><td>%d</td></tr>", pilot->secret);
        fprintf(f, "<tr><td>Only fight once</td><td>%d</td></tr>", pilot->only_fight_once);
        fprintf(f, "<tr><td>Req. Rank</td><td>%d</td></tr>", pilot->req_rank);
        fprintf(f, "<tr><td>Req. Max rank</td><td>%d</td></tr>", pilot->req_max_rank);
        fprintf(f, "<tr><td>Req. Fighter</td><td>%d</td></tr>", pilot->req_fighter);
        fprintf(f, "<tr><td>Req. Difficulty</td><td>%d</td></tr>", pilot->req_difficulty);
        fprintf(f, "<tr><td>Req. Enemy</td><td>%d</td></tr>", pilot->req_enemy);
        fprintf(f, "<tr><td>Req. Vitality</td><td>%d</td></tr>", pilot->req_vitality);
        fprintf(f, "<tr><td>Req. Accuracy</td><td>%d</td></tr>", pilot->req_accuracy);
        fprintf(f, "<tr><td>Req. Avg Damage</td><td>%d</td></tr>", pilot->req_avg_dmg);
        fprintf(f, "<tr><td>Req. Scrap</td><td>%d</td></tr>", pilot->req_scrap);
        fprintf(f, "<tr><td>Req. Destruction</td><td>%d</td></tr>", pilot->req_destroy);
        fprintf(f, "</table>");
        fprintf(f, "</div>");

        fprintf(f, "<div class=\"iblock\">");
        fprintf(f, "<h4>AI Opts</h4>");
        fprintf(f, "<table>");
        fprintf(f, "<tr><td>Attitude Normal</td><td>%d</td></tr>", pilot->att_normal);
        fprintf(f, "<tr><td>Attitude Hyper</td><td>%d</td></tr>", pilot->att_hyper);
        fprintf(f, "<tr><td>Attitude Jump</td><td>%d</td></tr>", pilot->att_jump);
        fprintf(f, "<tr><td>Attitude Def</td><td>%d</td></tr>", pilot->att_def);
        fprintf(f, "<tr><td>Attitude Sniper</td><td>%d</td></tr>", pilot->att_sniper);
        fprintf(f, "<tr><td>AP Throw</td><td>%d</td></tr>", pilot->ap_throw);
        fprintf(f, "<tr><td>AP Special</td><td>%d</td></tr>", pilot->ap_special);
        fprintf(f, "<tr><td>AP Jump</td><td>%d</td></tr>", pilot->ap_jump);
        fprintf(f, "<tr><td>AP High</td><td>%d</td></tr>", pilot->ap_high);
        fprintf(f, "<tr><td>AP Low</td><td>%d</td></tr>", pilot->ap_low);
        fprintf(f, "<tr><td>AP Middle</td><td>%d</td></tr>", pilot->ap_middle);

        fprintf(f, "<tr><td>Pref jump</td><td>%d</td></tr>", pilot->pref_jump);
        fprintf(f, "<tr><td>Pref fwd</td><td>%d</td></tr>", pilot->pref_fwd);
        fprintf(f, "<tr><td>Pref back</td><td>%d</td></tr>", pilot->pref_back);
        fprintf(f, "<tr><td>Learning</td><td>%f</td></tr>", pilot->learning);
        fprintf(f, "<tr><td>Forget</td><td>%f</td></tr>", pilot->forget);
        fprintf(f, "</table>");
        fprintf(f, "</div>");

        fprintf(f, "<div class=\"iblock\">");
        fprintf(f, "<h4>Other</h4>");
        fprintf(f, "<table>");
        fprintf(f, "<tr><td>Enemies (inc unranked)</td><td>%d</td></tr>", pilot->enemies_inc_unranked);
        fprintf(f, "<tr><td>Enemies (exl unranked)</td><td>%d</td></tr>", pilot->enemies_ex_unranked);

        fprintf(f, "<tr><td>Winnings</td><td>%d</td></tr>", pilot->winnings);
        fprintf(f, "<tr><td>Total value</td><td>%d</td></tr>", pilot->total_value);

        fprintf(f, "<tr><td>Photo ID</td><td>%d</td></tr>", pilot->photo_id);

        for(int m = 0; m < 10; m++) {
            char *quote = pilot->quotes[m];
            if(quote != NULL) {
                fprintf(f, "<tr><td>Quote %d:</td><td>%s</td></tr>", m, quote);
            }
        }
        fprintf(f, "</table>");
        fprintf(f, "</div>");
    }
}
