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

const char *PILOT_NAME[] = {
    "CRYSTAL",
    "STEFFAN",
    "MILANO",
    "CHRISTIAN",
    "SHIRRO",
    "JEAN-PAUL",
    "IBRAHIM",
    "ANGEL",
    "COSSETTE",
    "RAVEN",
    "KREISSACK"
};

const char *HAR_NAME[] = {
    "JAGUAR",
    "SHADOW",
    "THORN",
    "PYROS",
    "ELECTRA",
    "KATANA",
    "SHREDDER",
    "FLAIL",
    "GARGOYLE",
    "CHRONOS",
    "NOVA"
};

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("test_score <scores.dat> \n");
        return 0;
    }

    sd_score score;
    sd_score_create(&score);
    if(!sd_score_load(&score, argv[1])) {
        for(int page=0;page < sizeof(score.scores)/sizeof(score.scores[0]);page++) {
            printf("%s\n", PAGE_NAME[page]);
            printf("-----------\n");
            for(int i=0;i < sizeof(score.scores[0])/sizeof(score.scores[0][0]);i++) {
                sd_score_entry *e = &score.scores[page][i];
                const char *pilot_name = e->pilot_id < sizeof(PILOT_NAME)/sizeof(PILOT_NAME[0]) ? PILOT_NAME[e->pilot_id] : "(null)";
                const char *har_name = e->har_id < sizeof(HAR_NAME)/sizeof(HAR_NAME[0]) ? HAR_NAME[e->har_id] : "(null)";
                if(strlen(e->name) > 0) {
                    printf("%s SCORE:%u HAR_ID:%s[%u] PILOT_ID:%s[%u] PADDING:%u\n",
                           e->name, e->score, har_name, e->har_id, pilot_name, e->pilot_id, e->padding);
                } else {
                    break;
                }
            }
            printf("\n\n");
        }
    } else {
        printf("Could not open %s\n", argv[1]);
        return 1;
    }

    sd_score_free(&score);

    return 0;
}
