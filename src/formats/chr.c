#include <stdlib.h>
#include <string.h>

#include "formats/chr.h"
#include "formats/tournament.h"
#include "formats/pic.h"
#include "formats/error.h"
#include "formats/internal/memreader.h"
#include "formats/internal/memwriter.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "game/gui/pilotpic.h"
#include "game/common_defines.h"
#include "resources/ids.h"
#include "resources/pathmanager.h"
#include "resources/trnmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"

int sd_chr_create(sd_chr_file *chr) {
    if(chr == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(chr, 0, sizeof(sd_chr_file));
    return SD_SUCCESS;
}

int sd_chr_from_trn(sd_chr_file *chr, sd_tournament_file *trn, sd_pilot *pilot) {
    int ranked = 0;
    for (int i = 0; i < trn->enemy_count; i++) {
        chr->enemies[i] = omf_calloc(1, sizeof(sd_chr_enemy));
        sd_pilot_create(&chr->enemies[i]->pilot);
        memcpy(&chr->enemies[i]->pilot, trn->enemies[i], sizeof(sd_pilot));
        if (!trn->enemies[i]->secret) {
            ranked++;
            chr->enemies[i]->pilot.rank = ranked;
        }
    }
    palette_create(&chr->pal);
    chr->pilot.enemies_inc_unranked = trn->enemy_count;
    chr->pilot.enemies_ex_unranked = ranked;
    chr->pilot.rank = ranked + 1;
    strncpy(chr->pilot.trn_name, trn->filename, sizeof(chr->pilot.trn_name));
    strncpy(chr->pilot.trn_desc, trn->locales[0]->title, sizeof(chr->pilot.trn_desc));
    strncpy(chr->pilot.trn_image, trn->pic_file, sizeof(chr->pilot.trn_image));
    chr->photo = omf_calloc(1, sizeof(sd_sprite));
    sd_sprite_create(chr->photo);
    pilotpic_load(chr->photo, &chr->pal, PIC_PLAYERS, pilot->photo_id);
    return SD_SUCCESS;
}

int sd_chr_load(sd_chr_file *chr, const char *filename) {
    if(chr == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(filename);

    const char *dirname = pm_get_local_path(RESOURCE_PATH);

    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }
    
    // Read up pilot block and the unknown data
    memreader *mr = memreader_open_from_reader(r, 448);
    memreader_xor(mr, 0xAC);
    sd_pilot_create(&chr->pilot);
    sd_pilot_load_from_mem(mr, &chr->pilot);
    memreader_close(mr);

    char tmp[200];
    str pic_file;
    sd_tournament_file trn;
    sd_pic_file pic;

    if (dirname) {
        str_from_c(&pic_file, chr->pilot.trn_image);
        str_toupper(&pic_file);
        snprintf(tmp, 200, "%s%s", dirname, str_c(&pic_file));
        str_free(&pic_file);
        sd_pic_create(&pic);
        sd_pic_load(&pic, tmp);

        trn_load(&trn, chr->pilot.trn_name);
        for (int i = 0; i < 10; i++) {
            if (trn.locales[0]->end_texts[0][i]) {
                chr->cutscene_text[i] = omf_calloc(1, strlen(trn.locales[0]->end_texts[0][i]));
                strcpy(chr->cutscene_text[i], trn.locales[0]->end_texts[0][i]);
            }
        }
        // TODO do something better here
        if (strcmp("north_am.bk", trn.bk_name) == 0) {
            chr->cutscene = SCENE_NORTHAM;
        } else if (strcmp("katushai.bk", trn.bk_name) == 0) {
            chr->cutscene = SCENE_KATUSHAI;
        } else if (strcmp("war.bk", trn.bk_name) == 0) {
            chr->cutscene = SCENE_WAR;
        } else if (strcmp("world.bk", trn.bk_name) == 0) {
            chr->cutscene = SCENE_WORLD;
        } else {
            // fallback to something sane
            chr->cutscene = SCENE_VS;
        }
    }

    // Read enemies block
    mr = memreader_open_from_reader(r, 68 * chr->pilot.enemies_inc_unranked);
    memreader_xor(mr, (chr->pilot.enemies_inc_unranked * 68) & 0xFF);

    // Handle enemy data
    for(int i = 0; i < chr->pilot.enemies_inc_unranked; i++) {
        // Reserve & zero out
        chr->enemies[i] = omf_calloc(1, sizeof(sd_chr_enemy));
        sd_pilot_create(&chr->enemies[i]->pilot);
        sd_pilot_load_player_from_mem(mr, &chr->enemies[i]->pilot);
        if (dirname) {
            memcpy(&chr->enemies[i]->pilot.palette, &pic.photos[trn.enemies[i]->photo_id]->pal, sizeof(palette));
            chr->enemies[i]->pilot.photo = omf_calloc(1, sizeof(sd_sprite));
            sd_sprite_copy(chr->enemies[i]->pilot.photo,  pic.photos[trn.enemies[i]->photo_id]->sprite);
        }
        memread_buf(mr, chr->enemies[i]->unknown, 25);
        for (int m =0; m < 10; m++) {
            if (dirname && trn.enemies[i]->quotes[m]) {
                DEBUG("allocating %d bytes for quote %s", strlen(trn.enemies[i]->quotes[m]), trn.enemies[i]->quotes[m]);
                chr->enemies[i]->pilot.quotes[m] = omf_calloc(1, strlen(trn.enemies[i]->quotes[m]) + 1);
                strcpy(chr->enemies[i]->pilot.quotes[m], trn.enemies[i]->quotes[m]);
            }
        }
    }

    // Close memory reader for enemy data block
    memreader_close(mr);

    // Read HAR palette
    palette_create(&chr->pal);
    palette_load_range(r, &chr->pal, 0, 48);

    // No idea what this is.
    // TODO: Find out.
    chr->unknown_b = sd_read_udword(r);

    // Load sprite
    chr->photo = omf_calloc(1, sizeof(sd_sprite));
    sd_sprite_create(chr->photo);
    int ret = sd_sprite_load(r, chr->photo);
    if(ret != SD_SUCCESS) {
        goto error_1;
    }

    // Fix photo size
    chr->photo->width++;
    chr->photo->height++;

    // Close & return
    sd_reader_close(r);
    
    return SD_SUCCESS;

error_1:
    for(int i = 0; i < chr->pilot.enemies_inc_unranked; i++) {
        if(chr->enemies[i] != NULL) {
            omf_free(chr->enemies[i]);
        }
    }
    sd_sprite_free(chr->photo);
    sd_reader_close(r);
    return SD_FILE_PARSE_ERROR;
}

int sd_chr_save(sd_chr_file *chr, const char *filename) {
    if(chr == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // Save pilot and unknown
    memwriter *mw = memwriter_open();
    sd_pilot_save_to_mem(mw, &chr->pilot);
    memwriter_xor(mw, 0xAC);
    memwriter_save(mw, w);
    memwriter_close(mw);

    // TODO why did I have to add this
    sd_writer_seek_cur(w, 20);

    mw = memwriter_open();
    for(int i = 0; i < chr->pilot.enemies_inc_unranked; i++) {
        sd_pilot_save_player_to_mem(mw, &chr->enemies[i]->pilot);
        memwrite_buf(mw, chr->enemies[i]->unknown, 25);
    }
    memwriter_xor(mw, (chr->pilot.enemies_inc_unranked * 68) & 0xFF);
    memwriter_save(mw, w);
    memwriter_close(mw);

    // Save palette
    palette_save_range(w, &chr->pal, 0, 48);

    // Save this, whatever this is.
    sd_write_udword(w, chr->unknown_b);

    // Save photo. Hacky size fix.
    chr->photo->width--;
    chr->photo->height--;

    if (SD_SUCCESS != sd_sprite_save(w, chr->photo)) {
        return SD_FILE_WRITE_ERROR;
    }
    chr->photo->width++;
    chr->photo->height++;

    // Close & return
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_chr_free(sd_chr_file *chr) {
    for(int i = 0; i < chr->pilot.enemies_inc_unranked; i++) {
        if(chr->enemies[i] != NULL) {
            omf_free(chr->enemies[i]);
        }
    }
    sd_sprite_free(chr->photo);
}

const sd_chr_enemy *sd_chr_get_enemy(sd_chr_file *chr, int enemy_num) {
    if(chr == NULL || enemy_num < 0 || enemy_num >= chr->pilot.enemies_inc_unranked) {
        return NULL;
    }
    return chr->enemies[enemy_num];
}
