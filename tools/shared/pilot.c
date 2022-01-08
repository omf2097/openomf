#include "pilot.h"
#include <stdio.h>

static const char *har_list[] = {"Jaguar",   "Shadow", "Thorn",    "Pyros",   "Electra", "Katana",
                                 "Shredder", "Flail",  "Gargoyle", "Chronos", "Nova"};

static const char *difficulty_names[] = {
    "Aluminum",
    "Iron",
    "Steel",
    "Heavy Metal",
};

void print_bytes(char *buf, int len, int line, int padding) {
    for (int k = 0; k < padding; k++) {
        printf(" ");
    }
    for (int i = 1; i <= len; i++) {
        printf("%02x ", (uint8_t)buf[i - 1]);
        if (i % line == 0) {
            if (len != i) {
                printf("\n");
            }
            for (int k = 0; k < padding; k++) {
                printf(" ");
            }
        }
    }
}

void print_pilot_array_header() {
    printf("ID Name          Wins Loss HAR      Money   AP LP AS LS AR SR PW AG EN OFF   DEF   C1  "
           "C2  C3  Secret Photo Value\n");
}

void print_pilot_array_row(sd_pilot *pilot, int i) {
    const char *har_name = "Random";
    if (pilot->har_id < 255) {
        har_name = har_list[pilot->har_id];
    }
    printf("%2d %-13s %-4d %-4d %-8s %-7d %-2d %-2d %-2d %-2d %-2d %-2d %-2d %-2d %-2d %-5d %-5d "
           "%-3d %-3d %-3d %-6d %-5u %-7u\n",
           i, pilot->name, pilot->wins, pilot->losses, har_name, pilot->money, pilot->arm_power,
           pilot->leg_power, pilot->arm_speed, pilot->leg_speed, pilot->armor,
           pilot->stun_resistance, pilot->power, pilot->agility, pilot->endurance, pilot->offense,
           pilot->defense, pilot->color_1, pilot->color_2, pilot->color_3, pilot->secret,
           pilot->photo_id, pilot->total_value);
}

void print_pilot_player_info(sd_pilot *pilot) {
    if (pilot) {
        printf("  - Name:            %s\n", pilot->name);
        printf("  - Wins:            %d\n", pilot->wins);
        printf("  - Losses:          %d\n", pilot->losses);
        printf("  - Rank:            %d\n", pilot->rank);
        if (pilot->har_id == 255)
            printf("  - Har:             Random\n");
        else
            printf("  - Har:             %s\n", har_list[pilot->har_id]);
        printf("  - Arm Power:       %d\n", pilot->arm_power);
        printf("  - Leg Power:       %d\n", pilot->leg_power);
        printf("  - Arm Speed:       %d\n", pilot->arm_speed);
        printf("  - Leg Speed:       %d\n", pilot->leg_speed);
        printf("  - Armor:           %d\n", pilot->armor);
        printf("  - Stun Res.:       %d\n", pilot->stun_resistance);
        printf("  - Power:           %d\n", pilot->power);
        printf("  - Agility:         %d\n", pilot->agility);
        printf("  - Endurance:       %d\n", pilot->endurance);
        printf("  - Offense:         %d\n", pilot->offense);
        printf("  - Defense:         %d\n", pilot->defense);
        printf("  - Money:           %d\n", pilot->money);
        printf("  - Color:           %d,%d,%d\n", pilot->color_1, pilot->color_2, pilot->color_3);
    }
}

void print_pilot_info(sd_pilot *pilot) {
    if (pilot != NULL) {
        printf("### Pilot header for %s:\n", pilot->name);

        print_pilot_player_info(pilot);

        printf("  - TRN Name:        %s\n", pilot->trn_name);
        printf("  - TRN Desc:        %s\n", pilot->trn_desc);
        printf("  - TRN Image:       %s\n", pilot->trn_image);
        printf("  - Unk. Float C:    %f\n", pilot->unk_f_c);
        printf("  - Unk. Float D:    %f\n", pilot->unk_f_d);
        printf("  - Pilot ID:        %d\n", pilot->pilot_id);
        printf("  - Unknown K:       %d\n", pilot->unknown_k);
        printf("  - Force arena:     %d\n", pilot->force_arena);
        printf("  - Difficulty:      %s\n", difficulty_names[pilot->difficulty]);
        printf("  - unk_block_b:     ");
        print_bytes(pilot->unk_block_b, sizeof(pilot->unk_block_b), 16, 0);
        printf("\n");
        printf("  - Movement:        %d\n", pilot->movement);
        printf("  - unk_block_c:\n");
        for (int i = 0; i < 3; i++) {
            printf("     [%d] = %d\n", i, pilot->unk_block_c[i]);
        }

        printf("  - Enhancements:\n");
        for (int i = 0; i < 11; i++) {
            printf("     * %-10s: %x\n", har_list[i], pilot->enhancements[i]);
        }

        printf("  - Secret:          %d\n", pilot->secret);
        printf("  - Only fight once: %d\n", pilot->only_fight_once);
        printf("  - Requirements:\n");
        printf("    * Rank:          %d\n", pilot->req_rank);
        printf("    * Max rank:      %d\n", pilot->req_max_rank);
        printf("    * Fighter:       %d\n", pilot->req_fighter);
        printf("    * Difficulty:    %d\n", pilot->req_difficulty);
        printf("    * Enemy:         %d\n", pilot->req_enemy);
        printf("    * Vitality:      %d\n", pilot->req_vitality);
        printf("    * Accuracy:      %d\n", pilot->req_accuracy);
        printf("    * Avg Damage:    %d\n", pilot->req_avg_dmg);
        printf("    * Scrap:         %d\n", pilot->req_scrap);
        printf("    * Destruction    %d\n", pilot->req_destroy);

        printf("  - Attitude:\n");
        printf("    * Normal:        %d\n", pilot->att_normal);
        printf("    * Hyper:         %d\n", pilot->att_hyper);
        printf("    * Jump:          %d\n", pilot->att_jump);
        printf("    * Def:           %d\n", pilot->att_def);
        printf("    * Sniper:        %d\n", pilot->att_sniper);

        printf("  - unk_block_d:\n");
        for (int i = 0; i < 3; i++) {
            printf("     [%d] = %d\n", i, pilot->unk_block_d[i]);
        }

        printf("  - AP Throw:        %d\n", pilot->ap_throw);
        printf("  - AP Special:      %d\n", pilot->ap_special);
        printf("  - AP Jump:         %d\n", pilot->ap_jump);
        printf("  - AP High:         %d\n", pilot->ap_high);
        printf("  - AP Low:          %d\n", pilot->ap_low);
        printf("  - AP Middle:       %d\n", pilot->ap_middle);

        printf("  - Pref jump:       %d\n", pilot->pref_jump);
        printf("  - Pref fwd:        %d\n", pilot->pref_fwd);
        printf("  - Pref back:       %d\n", pilot->pref_back);

        printf("  - Unknown E:       %d\n", pilot->unknown_e);
        printf("  - Learning:        %f\n", pilot->learning);
        printf("  - Forget:          %f\n", pilot->forget);

        printf("  - unk_block_f:     ");
        print_bytes(pilot->unk_block_f, sizeof(pilot->unk_block_f), 26, 0);
        printf("\n");

        printf("  - Enemies (inc unranked): %d\n", pilot->enemies_inc_unranked);
        printf("  - Enemies (exl unranked): %d\n", pilot->enemies_ex_unranked);

        printf("  - Unk. Int A:      %d\n", pilot->unk_d_a);
        printf("  - Unk. Int B:      %d\n", pilot->unk_d_b);

        printf("  - Winnings:        %d\n", pilot->winnings);
        printf("  - Total value:     %d\n", pilot->total_value);

        printf("  - Unk. Float A:    %f\n", pilot->unk_f_a);
        printf("  - Unk. Float B:    %f\n", pilot->unk_f_b);

        printf("  - Palette:\n");
        print_bytes((char *)pilot->palette.data, 144, 16, 4);
        printf("\n");

        printf("  - Unknown i        %d\n", pilot->unk_block_i);
        printf("  - Photo ID         %d\n", pilot->photo_id);

        printf("  - Quotes:\n");
        for (int m = 0; m < 10; m++) {
            char *quote = pilot->quotes[m];
            if (quote != NULL) {
                printf("    * %s\n", quote);
            }
        }
    }
}
