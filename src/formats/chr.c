#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats/chr.h"
#include "formats/error.h"
#include "formats/internal/memreader.h"
#include "formats/internal/memwriter.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "formats/pic.h"
#include "formats/tournament.h"
#include "game/common_defines.h"
#include "game/gui/pilotpic.h"
#include "resources/pathmanager.h"
#include "resources/trnmanager.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
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
    for(uint32_t i = 0; i < trn->enemy_count; i++) {
        chr->enemies[i] = omf_calloc(1, sizeof(sd_chr_enemy));
        sd_pilot_create(&chr->enemies[i]->pilot);
        sd_pilot_clone(&chr->enemies[i]->pilot, trn->enemies[i]);
        if(!trn->enemies[i]->secret) {
            ranked++;
            chr->enemies[i]->pilot.rank = ranked;
        }
    }
    vga_palette_init(&chr->pal);
    chr->pilot.enemies_inc_unranked = trn->enemy_count;
    chr->pilot.enemies_ex_unranked = ranked;
    chr->pilot.rank = ranked + 1;
    strncpy_or_truncate(chr->pilot.trn_name, trn->filename, sizeof(chr->pilot.trn_name));
    strncpy_or_truncate(chr->pilot.trn_desc, trn->locales[0]->title, sizeof(chr->pilot.trn_desc));
    strncpy_or_truncate(chr->pilot.trn_image, trn->pic_file, sizeof(chr->pilot.trn_image));
    chr->photo = omf_calloc(1, sizeof(sd_sprite));
    sd_sprite_copy(chr->photo, pilot->photo);
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
    str trn_file;
    sd_tournament_file trn;
    sd_pic_file pic;
    bool trn_loaded = false;

    if(dirname) {
        str_from_c(&pic_file, chr->pilot.trn_image);
        str_toupper(&pic_file);
        snprintf(tmp, 200, "%s%s", dirname, str_c(&pic_file));
        str_free(&pic_file);
        sd_pic_create(&pic);
        sd_pic_load(&pic, tmp);

        if(*chr->pilot.trn_name != '\0') {
            str_from_c(&trn_file, chr->pilot.trn_name);
            str_toupper(&trn_file);
            trn_loaded = trn_load(&trn, str_c(&trn_file)) == 0;
            str_free(&trn_file);
        }
    }

    if(trn_loaded) {
        for(int i = 0; i < 10; i++) {
            if(trn.locales[0]->end_texts[0][i]) {
                chr->cutscene_text[i] = omf_strdup(trn.locales[0]->end_texts[0][i]);
            }
        }
        // TODO do something better here
        if(strcmp("north_am.bk", trn.bk_name) == 0) {
            chr->cutscene = SCENE_NORTHAM;
        } else if(strcmp("katushai.bk", trn.bk_name) == 0) {
            chr->cutscene = SCENE_KATUSHAI;
        } else if(strcmp("war.bk", trn.bk_name) == 0) {
            chr->cutscene = SCENE_WAR;
        } else if(strcmp("world.bk", trn.bk_name) == 0) {
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
        if(chr->enemies[i]->pilot.har_id == 255) {
            // pick a random HAR
            chr->enemies[i]->pilot.har_id = rand_int(10);
        }
        if(trn_loaded) {
            memcpy(&chr->enemies[i]->pilot.palette, &pic.photos[trn.enemies[i]->photo_id]->pal, sizeof(vga_palette));
            chr->enemies[i]->pilot.photo = omf_calloc(1, sizeof(sd_sprite));
            sd_sprite_copy(chr->enemies[i]->pilot.photo, pic.photos[trn.enemies[i]->photo_id]->sprite);
            //  copy all the "pilot" fields (eg. winnings) over from the tournament file
            chr->enemies[i]->pilot.unk_f_c = trn.enemies[i]->unk_f_c;
            chr->enemies[i]->pilot.unk_f_d = trn.enemies[i]->unk_f_d;
            chr->enemies[i]->pilot.pilot_id = trn.enemies[i]->pilot_id;
            chr->enemies[i]->pilot.unknown_k = trn.enemies[i]->unknown_k;
            chr->enemies[i]->pilot.force_arena = trn.enemies[i]->force_arena;
            chr->enemies[i]->pilot.difficulty = trn.enemies[i]->difficulty;
            chr->enemies[i]->pilot.movement = trn.enemies[i]->movement;
            // chr->enemies[i]->pilot.unk_block_c = trn.enemies[i]->unk_block_c;
            memcpy(chr->enemies[i]->pilot.enhancements, trn.enemies[i]->enhancements,
                   sizeof(chr->enemies[i]->pilot.enhancements));
            chr->enemies[i]->pilot.secret = trn.enemies[i]->secret;
            chr->enemies[i]->pilot.only_fight_once = trn.enemies[i]->only_fight_once;
            chr->enemies[i]->pilot.req_rank = trn.enemies[i]->req_rank;
            chr->enemies[i]->pilot.req_max_rank = trn.enemies[i]->req_max_rank;
            chr->enemies[i]->pilot.req_fighter = trn.enemies[i]->req_fighter;
            chr->enemies[i]->pilot.req_difficulty = trn.enemies[i]->req_difficulty;
            chr->enemies[i]->pilot.req_enemy = trn.enemies[i]->req_enemy;
            chr->enemies[i]->pilot.req_vitality = trn.enemies[i]->req_vitality;
            chr->enemies[i]->pilot.req_accuracy = trn.enemies[i]->req_accuracy;
            chr->enemies[i]->pilot.req_avg_dmg = trn.enemies[i]->req_avg_dmg;
            chr->enemies[i]->pilot.req_scrap = trn.enemies[i]->req_scrap;
            chr->enemies[i]->pilot.req_destroy = trn.enemies[i]->req_destroy;
            chr->enemies[i]->pilot.att_normal = trn.enemies[i]->att_normal;
            chr->enemies[i]->pilot.att_hyper = trn.enemies[i]->att_hyper;
            chr->enemies[i]->pilot.att_def = trn.enemies[i]->att_def;
            chr->enemies[i]->pilot.att_sniper = trn.enemies[i]->att_sniper;
            chr->enemies[i]->pilot.ap_throw = trn.enemies[i]->ap_throw;
            chr->enemies[i]->pilot.ap_special = trn.enemies[i]->ap_special;
            chr->enemies[i]->pilot.ap_jump = trn.enemies[i]->ap_jump;
            chr->enemies[i]->pilot.ap_high = trn.enemies[i]->ap_high;
            chr->enemies[i]->pilot.ap_low = trn.enemies[i]->ap_low;
            chr->enemies[i]->pilot.ap_middle = trn.enemies[i]->ap_middle;
            chr->enemies[i]->pilot.pref_jump = trn.enemies[i]->pref_jump;
            chr->enemies[i]->pilot.pref_fwd = trn.enemies[i]->pref_fwd;
            chr->enemies[i]->pilot.pref_back = trn.enemies[i]->pref_back;
            chr->enemies[i]->pilot.unknown_e = trn.enemies[i]->unknown_e;
            chr->enemies[i]->pilot.learning = trn.enemies[i]->learning;
            chr->enemies[i]->pilot.forget = trn.enemies[i]->forget;
            // chr->enemies[i]->pilot.unk_block_f = trn.enemies[i]->unk_block_f;
            chr->enemies[i]->pilot.winnings = trn.enemies[i]->winnings;
            chr->enemies[i]->pilot.total_value = trn.enemies[i]->total_value;
            chr->enemies[i]->pilot.photo_id = trn.enemies[i]->photo_id;
        }
        memread_buf(mr, chr->enemies[i]->unknown, 25);
        for(int m = 0; m < 10; m++) {
            if(trn_loaded && trn.enemies[i]->quotes[m]) {
                chr->enemies[i]->pilot.quotes[m] = omf_strdup(trn.enemies[i]->quotes[m]);
            }
        }
    }

    if(trn_loaded) {
        sd_pic_free(&pic);
        sd_tournament_free(&trn);
    }

    // Close memory reader for enemy data block
    memreader_close(mr);

    // Read HAR palette
    vga_palette_init(&chr->pal);
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

    chr->pilot.photo = chr->photo;

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

    if(SD_SUCCESS != sd_sprite_save(w, chr->photo)) {
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
            if(chr->enemies[i]->pilot.photo) {
                sd_sprite_free(chr->enemies[i]->pilot.photo);
                omf_free(chr->enemies[i]->pilot.photo);
            }
            for(int m = 0; m < 10; m++) {
                if(chr->enemies[i]->pilot.quotes[m]) {
                    omf_free(chr->enemies[i]->pilot.quotes[m]);
                }
            }
            omf_free(chr->enemies[i]);
        }
    }
    for(int i = 0; i < 10; i++) {
        if(chr->cutscene_text[i]) {
            omf_free(chr->cutscene_text[i]);
        }
    }
    sd_sprite_free(chr->photo);
    omf_free(chr->photo);
}

const sd_chr_enemy *sd_chr_get_enemy(sd_chr_file *chr, int enemy_num) {
    if(chr == NULL || enemy_num < 0 || enemy_num >= chr->pilot.enemies_inc_unranked) {
        return NULL;
    }
    return chr->enemies[enemy_num];
}
