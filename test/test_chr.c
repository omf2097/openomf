#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <shadowdive/shadowdive.h>

const char *har_list[] = {
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

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("test_chr <chrfile>\n");
        return 0;
    }

    // Open pic file
    sd_chr_file *chr = sd_chr_create();
    int ret = sd_chr_load(chr, argv[1]);
    if(ret != SD_SUCCESS) {
        printf("Shadowdive error %d: %s\n", ret, sd_get_error(ret));
        return 1;
    } 

    // Dump file header data
    printf("Loaded file '%s'\n", argv[1]);
    printf("\n");
    printf("Player data:\n");
    printf(" - Name:      %s\n", chr->name);
    printf(" - Wins:      %d\n", chr->wins);
    printf(" - Losses:    %d\n", chr->losses);
    printf(" - Rank:      %d\n", chr->rank);
    printf(" - Har:       %s (%d)\n", har_list[chr->har], chr->har);
    printf(" - Arm Power: %d\n", chr->arm_power);
    printf(" - Leg Power: %d\n", chr->leg_power);
    printf(" - Arm Speed: %d\n", chr->arm_speed);
    printf(" - Leg Speed: %d\n", chr->leg_speed);
    printf(" - Armor:     %d\n", chr->armor);
    printf(" - Stun Res.: %d\n", chr->stun_resistance);
    printf(" - Power:     %d\n", chr->power);
    printf(" - Agility:   %d\n", chr->agility);
    printf(" - Endurance: %d\n", chr->endurance);
    printf(" - Credits:   %d\n", chr->credits);
    printf(" - Color:     (%d,%d,%d)\n", 
        chr->color_1,
        chr->color_2,
        chr->color_3);

    // Tournament section
    printf(" - TRN Name:  %s\n", chr->trn_name);
    printf(" - TRN Desc:  %s\n", chr->trn_desc);
    printf(" - TRN Image: %s\n", chr->trn_image);    
    printf(" - Diffic.:   %d\n", chr->difficulty);
    printf(" - Enemy ex:  %d\n", chr->enemies_ex_unranked);
    printf(" - Enemy inc: %d\n", chr->enemies_inc_unranked);

    // Enhancements
    printf(" - Enh:       ");
    for(int i = 0; i < 11; i++) {
        printf("%02X ", chr->enhancements[i]);
    }

    // Portrait data
    printf("\n");
    printf(" - Portrait:\n");
    printf("   * Size = (%d,%d)\n", 
        chr->photo->img->w, 
        chr->photo->img->h);
    printf("   * Position = (%d,%d)\n", 
        chr->photo->pos_x, 
        chr->photo->pos_y);
    printf("   * Length = %d\n", 
        chr->photo->img->len);

    // Enemy data
    printf("\nEnemies:\n");
    for(int i = 0; i < chr->enemies_inc_unranked; i++) {
        printf(" - [%d] %s\n", i, chr->enemies[i]->name);
        printf("   * Wins:    %d\n", chr->enemies[i]->wins);
        printf("   * Losses:  %d\n", chr->enemies[i]->losses);
        printf("   * Rank:    %d\n", chr->enemies[i]->rank);
        printf("   * Har:     %s (%d)\n", 
            har_list[chr->enemies[i]->har], 
            chr->enemies[i]->har);
    }

    sd_chr_delete(chr);
    return 0;
}
