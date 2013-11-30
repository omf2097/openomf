#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("test_tourn <tournamentfile>\n");
        return 0;
    }
    
    sd_tournament_file *trn = sd_tournament_create();
    if(sd_tournament_load(trn, argv[1])) {
        printf("Tournament file could not be loaded!\n");
        return 1;
    }
    
    
    sd_tournament_delete(trn);
    return 0;
}
