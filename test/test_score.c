#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <shadowdive/shadowdive.h>

const char *PAGE_NAME[] = {
    "ONE ROUND",
    "BEST 2 OF 3",
    "BEST 3 OF 5",
    "BEST 4 OF 7"
};
int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("test_score <scores.dat> \n");
        return 0;
    }

    sd_score *score = sd_score_create();

    if(!sd_score_load(score, argv[1])) {
        for(int page=0;page < 4;page++) {
            printf("%s\n", PAGE_NAME[page]);
            printf("-----------\n");
            for(int i=0;i < 20;i++) {
                sd_score_entry *e = &score->scores[page][i];
                if(strlen(e->name) > 0) {
                    printf("%s SCORE:%u PILOT_ID:%u\n", e->name, e->score, e->pilot_id);
                } else {
                    break;
                }
            }
            printf("\n\n");
        }
    } else {
        printf("Could not open scores.dat\n");
        return 1;
    }

    sd_score_delete(score);

    return 0;
}
