#include <stdlib.h>
#include <string.h>

#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/error.h"
#include "shadowdive/tournament.h"

int sd_tournament_create(sd_tournament_file *trn) {
    if(trn == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(trn, 0, sizeof(sd_tournament_file));
    return SD_SUCCESS;
}

static void free_enemies(sd_tournament_file *trn) {
    for(int i = 0; i < MAX_TRN_ENEMIES; i++) {
        if(trn->enemies[i]) {
            sd_pilot_free(trn->enemies[i]);
            free(trn->enemies[i]);
            trn->enemies[i] = NULL;
        }
        for(int k = 0; k < MAX_TRN_LOCALES; k++) {
            if(trn->quotes[i][k]) {
                free(trn->quotes[i][k]);
                trn->quotes[i][k] = NULL;
            }
        }
    }
}

static void free_locales(sd_tournament_file *trn) {
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        if(trn->locales[i]) {
            if(trn->locales[i]->logo)
                sd_sprite_free(trn->locales[i]->logo);
            if(trn->locales[i]->description)
                free(trn->locales[i]->description);
            if(trn->locales[i]->title)
                free(trn->locales[i]->title);
            for(int har = 0; har < 11; har++) {
                for(int page = 0; page < 10; page++) {
                    if(trn->locales[i]->end_texts[har][page])
                        free(trn->locales[i]->end_texts[har][page]);
                }
            }
            free(trn->locales[i]);
            trn->locales[i] = NULL;
        }
    }
}

char *read_variable_str(sd_reader *r) {
    int len = sd_read_uword(r);
    char *str = NULL;
    if(len > 0) {
        str = (char*)malloc(len);
        sd_read_buf(r, str, len);
    }
    return str;
}

int sd_tournament_load(sd_tournament_file *trn, const char *filename) {
    if(trn == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Make sure that the file looks at least relatively okay
    // TODO: Add other checks.
    if(sd_reader_filesize(r) < 1582) {
        goto error_0;
    }

    // Read tournament data
    trn->enemy_count = sd_read_dword(r);
    int victory_text_offset = sd_read_dword(r);
    sd_read_buf(r, trn->bk_name, 14);
    trn->winnings_multiplier = sd_read_float(r);
    sd_skip(r, 4);
    trn->registration_free = sd_read_dword(r);
    trn->assumed_initial_value = sd_read_dword(r);
    trn->tournament_id = sd_read_dword(r);

    // Read enemy block offsets
    sd_reader_set(r, 300);
    int offset_list[256]; // Should be large enough
    for(int i = 0; i < trn->enemy_count + 1; i++) {
        offset_list[i] = sd_read_dword(r);
    }

    // Read enemy data
    for(int i = 0; i < trn->enemy_count; i++) {
        trn->enemies[i] = malloc(sizeof(sd_pilot));
        if(trn->enemies[i] == NULL) {
            return SD_OUT_OF_MEMORY;
        }

        // Find data length
        sd_reader_set(r, offset_list[i]);

        // Read enemy pilot information
        sd_pilot_create(trn->enemies[i]);
        sd_pilot_load(r, trn->enemies[i]);

        // Read quotes
        for(int m = 0; m < MAX_TRN_LOCALES; m++) {
            trn->quotes[i][m] = read_variable_str(r);
        }

        // Check for errors
        if(!sd_reader_ok(r)) {
            goto error_1;
        }
    }

    // Seek sprite start offset
    sd_reader_set(r, offset_list[trn->enemy_count]);

    // Allocate locales
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        trn->locales[i] = malloc(sizeof(sd_tournament_locale));
        trn->locales[i]->logo = NULL;
        trn->locales[i]->description = NULL;
        trn->locales[i]->title = NULL;
        for(int har = 0; har < 11; har++) {
            for(int page = 0; page < 10; page++) {
                trn->locales[i]->end_texts[har][page] = NULL;
            }
        }
    }

    // Load logos to locales
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        trn->locales[i]->logo = malloc(sizeof(sd_sprite));
        sd_sprite_create(trn->locales[i]->logo);
        if(sd_sprite_load(r, trn->locales[i]->logo) != SD_SUCCESS) {
            goto error_2;
        }
    }

    // Read palette. Only 40 colors are defined, starting
    // from palette position 128. Remember to convert VGA pal.
    memset((void*)&trn->pal, 0, sizeof(sd_palette));
    char d[3];
    for(int i = 128; i < 168; i++) {
        sd_read_buf(r, d, 3);
        trn->pal.data[i][0] = ((d[0] << 2) | (d[0] >> 4));
        trn->pal.data[i][1] = ((d[1] << 2) | (d[1] >> 4));
        trn->pal.data[i][2] = ((d[2] << 2) | (d[2] >> 4));
    }

    // Read pic filename
    trn->pic_file = read_variable_str(r);

    // Read tournament descriptions
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        trn->locales[i]->title = read_variable_str(r);
        trn->locales[i]->description = read_variable_str(r);
    }

    // Make sure we are in correct position
    if(sd_reader_pos(r) != victory_text_offset) {
        goto error_2;
    }

    // Load texts
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        for(int har = 0; har < 11; har++) {
            for(int page = 0; page < 10; page++) {
                trn->locales[i]->end_texts[har][page] = read_variable_str(r);
            }
        }
    }

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;

error_2:
    free_locales(trn);

error_1:
    free_enemies(trn);

error_0:
    sd_reader_close(r);
    return SD_FILE_PARSE_ERROR;
}

int sd_tournament_save(sd_tournament_file *trn, const char *filename) {
    if(trn == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }
    return SD_FILE_OPEN_ERROR;
}

void sd_tournament_free(sd_tournament_file *trn) {
    if(trn == NULL) return;
    free_locales(trn);
    free_enemies(trn);
    if(trn->pic_file != NULL) {
        free(trn->pic_file);
    }
}
