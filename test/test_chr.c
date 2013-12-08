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
    if(argc < 2 || argc == 3 || argc == 4) {
        printf("test_chr <chrfile> [-d filename.ppm palettefile.bk]\n");
        return 0;
    }

    // Get output dump path
    int dump = 0;
    char *dumpout = NULL;
    char *bkfile = NULL;
    if(argc == 5) {
        if(strcmp(argv[2], "-d") == 0) {
            dump = 1;
            dumpout = argv[3];
            bkfile = argv[4];
        }
    }

    // Open pic file
    sd_chr_file *chr = sd_chr_create();
    int ret = sd_chr_load(chr, argv[1]);
    if(ret != SD_SUCCESS) {
        printf("Shadowdive error %d: %s\n", ret, sd_get_error(ret));
        return 1;
    } 

    printf("Loaded file '%s'\n", argv[1]);

    if(dump) {
        sd_bk_file *bk = sd_bk_create();
        int ret = sd_bk_load(bk, bkfile);
        if(ret != SD_SUCCESS) {
            printf("Could not load BK file! %s (%d)\n", sd_get_error(ret), ret);
            
            return 1;
        } else {
            printf("Loaded file '%s'.\n", bkfile);
        }

        printf("Dumping portrait to '%s'!\n", dumpout);
        sd_rgba_image *img = sd_sprite_image_decode(
            chr->photo->img, bk->palettes[0], -1);
        sd_rgba_image_to_ppm(img, dumpout);
        sd_rgba_image_delete(img);

        sd_bk_delete(bk);
        sd_chr_delete(chr);
        return 0;
    }

    // Dump file header data
    
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
    printf("   * I/M = %u/%u\n",
        chr->photo->index,
        chr->photo->missing);

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
