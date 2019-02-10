#include <string.h>
#include "formats/score.h"
#include "formats/error.h"
#include "utils/log.h"
#include "resources/pathmanager.h"
#include "resources/scores.h"

void scores_clear(scoreboard *sb) {
    for(int i = 0; i < 4; i++) {
        for(int m = 0; m < 20; m++) {
            sb->entries[i][m].score = 0;
            sb->entries[i][m].har_id = 0;
            sb->entries[i][m].pilot_id = 0;
            strncpy(sb->entries[i][m].name, "", 16);
        }
    }
}

int scores_read(scoreboard *sb) {
    sd_score score_file;
    if(sd_score_create(&score_file) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_score_load(&score_file, pm_get_local_path(SCORE_PATH)) != SD_SUCCESS) {
        PERROR("Failure while attempting to open scores file!");
        goto error_1;
    }
    DEBUG("Loaded scores file successfully!");

    // Fetch data
    for(int i = 0; i < 4; i++) {
        for(int m = 0; m < 20; m++) {
            sb->entries[i][m].score = score_file.scores[i][m].score;
            sb->entries[i][m].har_id = score_file.scores[i][m].har_id;
            sb->entries[i][m].pilot_id = score_file.scores[i][m].pilot_id;
            strncpy(sb->entries[i][m].name, score_file.scores[i][m].name, 16);
        }
    }

    // All done, free raw data
    sd_score_free(&score_file);
    return 0;

error_1:
    sd_score_free(&score_file);
error_0:
    return 1;
}

int scores_write(scoreboard *sb) {
    sd_score score_file;
    if(sd_score_create(&score_file) != SD_SUCCESS) {
        return 1;
    }

    // Convert data
    for(int i = 0; i < 4; i++) {
        for(int m = 0; m < 20; m++) {
            score_file.scores[i][m].score = sb->entries[i][m].score;
            score_file.scores[i][m].har_id = sb->entries[i][m].har_id;
            score_file.scores[i][m].pilot_id = sb->entries[i][m].pilot_id;
            strncpy(score_file.scores[i][m].name, sb->entries[i][m].name, 16);
        }
    }

    // Save
    sd_score_save(&score_file, pm_get_local_path(SCORE_PATH));

    // All done
    sd_score_free(&score_file);
    return 0;
}
