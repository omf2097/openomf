#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats/error.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "formats/tournament.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"

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
            omf_free(trn->enemies[i]);
        }
    }
}

static void free_locales(sd_tournament_file *trn) {
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        if(trn->locales[i]) {
            if(trn->locales[i]->logo) {
                sd_sprite_free(trn->locales[i]->logo);
                omf_free(trn->locales[i]->logo);
            }
            if(trn->locales[i]->description) {
                omf_free(trn->locales[i]->description);
            }
            if(trn->locales[i]->stripped_description) {
                omf_free(trn->locales[i]->stripped_description);
            }
            if(trn->locales[i]->title) {
                omf_free(trn->locales[i]->title);
            }
            for(int har = 0; har < 11; har++) {
                for(int page = 0; page < 10; page++) {
                    if(trn->locales[i]->end_texts[har][page]) {
                        omf_free(trn->locales[i]->end_texts[har][page]);
                    }
                }
            }
            omf_free(trn->locales[i]);
        }
    }
}

int sd_tournament_set_bk_name(sd_tournament_file *trn, const char *bk_name) {
    if(trn == NULL || bk_name == NULL) {
        return SD_INVALID_INPUT;
    }
    snprintf(trn->bk_name, sizeof(trn->bk_name), "%s", bk_name);
    return SD_SUCCESS;
}

int sd_tournament_set_pic_name(sd_tournament_file *trn, const char *pic_name) {
    if(trn == NULL || pic_name == NULL) {
        return SD_INVALID_INPUT;
    }
    size_t len = strlen(pic_name) + 1;
    trn->pic_file = omf_realloc(trn->pic_file, len);
    snprintf(trn->pic_file, len, "%s", pic_name);
    return SD_SUCCESS;
}

static void parse_tournament_description(sd_tournament_locale *locale) {
    int width = 320, center = 0, vmove = 0, size = -1, color = -1;
    const char *desc = locale->description;
    const char *end = desc;
    char *ptr = NULL;
    ptr = strstr(desc, "{WIDTH ");
    if(ptr != NULL) {
        sscanf(ptr, "{WIDTH %d}", &width);
        if(ptr > end) {
            end = ptr;
        }
    }
    ptr = strstr(desc, "{CENTER ");
    if(ptr != NULL) {
        sscanf(ptr, "{CENTER %d}", &center);
        if(ptr > end) {
            end = ptr;
        }
    }
    ptr = strstr(desc, "{VMOVE ");
    if(ptr != NULL) {
        sscanf(ptr, "{VMOVE %d}", &vmove);
        if(ptr > end) {
            end = ptr;
        }
    }
    ptr = strstr(desc, "{SIZE ");
    if(ptr != NULL) {
        sscanf(ptr, "{SIZE %d}", &size);
        if(ptr > end) {
            end = ptr;
        }
    }
    ptr = strstr(desc, "{COLOR ");
    if(ptr != NULL) {
        sscanf(ptr, "{COLOR %d}", &color);
        if(ptr > end) {
            end = ptr;
        }
    }
    const char *start = strchr(end, '}');
    if(start == NULL) {
        start = desc;
    } else {
        start++;
    }
    locale->desc_width = width;
    locale->desc_center = center;
    locale->desc_vmove = vmove;
    locale->desc_size = size;
    locale->desc_color = color;
    // Ignore the rest of the possible metadata.
    ptr = strchr(start, '{');
    if(ptr != NULL) {
        locale->stripped_description = omf_strndup(start, ptr - start);
    } else {
        locale->stripped_description = omf_strdup(start);
    }
}

int sd_tournament_load(sd_tournament_file *trn, const char *filename) {
    int ret = SD_FILE_PARSE_ERROR;
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

    // Read enemy count and make sure it seems somwhat correct
    uint32_t enemy_count = sd_read_udword(r);
    if(enemy_count >= MAX_TRN_ENEMIES || enemy_count == 0) {
        goto error_0;
    }

    char *justfile = strrchr(filename, pm_path_sep);
    if(justfile == NULL) {
        strncpy_or_truncate(trn->filename, filename, sizeof(trn->filename));
    } else {
        strncpy_or_truncate(trn->filename, justfile + 1, sizeof(trn->filename));
    }
    trn->enemy_count = enemy_count;

    // Read tournament data
    int victory_text_offset = sd_read_dword(r);
    sd_read_buf(r, trn->bk_name, 14);
    trn->winnings_multiplier = sd_read_float(r);
    trn->unknown_a = sd_read_dword(r);
    trn->registration_fee = sd_read_dword(r);
    trn->assumed_initial_value = sd_read_dword(r);
    trn->tournament_id = sd_read_dword(r);

    // Read enemy block offsets
    sd_reader_set(r, 300);
    int offset_list[MAX_TRN_ENEMIES + 2]; // Should be large enough
    memset(offset_list, 0, sizeof(offset_list));
    for(unsigned i = 0; i < trn->enemy_count + 1; i++) {
        offset_list[i] = sd_read_dword(r);
    }

    // Read enemy data
    for(unsigned i = 0; i < trn->enemy_count; i++) {
        trn->enemies[i] = omf_calloc(1, sizeof(sd_pilot));

        // Find data length
        sd_reader_set(r, offset_list[i]);

        // Read enemy pilot information
        sd_pilot_create(trn->enemies[i]);
        sd_pilot_load(r, trn->enemies[i]);

        // Check for errors
        if(!sd_reader_ok(r)) {
            goto error_1;
        }
    }

    // Seek sprite start offset
    sd_reader_set(r, offset_list[trn->enemy_count]);

    // Allocate locales
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        trn->locales[i] = omf_calloc(1, sizeof(sd_tournament_locale));
    }

    // Load logos to locales
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        trn->locales[i]->logo = omf_calloc(1, sizeof(sd_sprite));
        sd_sprite_create(trn->locales[i]->logo);
        if((ret = sd_sprite_load(r, trn->locales[i]->logo)) != SD_SUCCESS) {
            goto error_2;
        }
    }

    // Read palette. Only 40 colors are defined, starting
    // from palette position 128. Remember to convert VGA pal.
    vga_palette_init(&trn->pal);
    palette_load_range(r, &trn->pal, 128, 40);

    // Read pic filename
    trn->pic_file = sd_read_variable_str(r);

    // Read tournament descriptions
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        trn->locales[i]->title = sd_read_variable_str(r);
        trn->locales[i]->description = sd_read_variable_str(r);
        parse_tournament_description(trn->locales[i]);
    }

    // Make sure we are in correct position
    if(sd_reader_pos(r) != victory_text_offset) {
        ret = SD_FILE_PARSE_ERROR;
        goto error_2;
    }

    // Load texts
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        for(int har = 0; har < 11; har++) {
            for(int page = 0; page < 10; page++) {
                trn->locales[i]->end_texts[har][page] = sd_read_variable_str(r);
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
    return ret;
}

int sd_tournament_save(const sd_tournament_file *trn, const char *filename) {
    if(trn == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // Header
    sd_write_udword(w, trn->enemy_count);
    sd_write_dword(w, 0); // Write this later!
    sd_write_buf(w, trn->bk_name, 14);
    sd_write_float(w, trn->winnings_multiplier);
    sd_write_dword(w, trn->unknown_a);
    sd_write_dword(w, trn->registration_fee);
    sd_write_dword(w, trn->assumed_initial_value);
    sd_write_dword(w, trn->tournament_id);

    // Write null until offset 300
    // Nothing of consequence here.
    sd_write_fill(w, 0, 300 - sd_writer_pos(w));

    // Write first offset
    sd_write_udword(w, 1100);

    // Write null until offset 1100
    // Nothing of consequence here.
    sd_write_fill(w, 0, 1100 - sd_writer_pos(w));

    // Walk through the enemies list, and write
    // offsets and blocks as we go
    for(unsigned i = 0; i < trn->enemy_count; i++) {
        // Save pilot
        sd_pilot_save(w, trn->enemies[i]);

        // Update catalog
        long c_pos = sd_writer_pos(w);
        if(c_pos < 0) {
            goto error;
        }
        if(sd_writer_seek_start(w, 300 + (i + 1) * 4) < 0) {
            goto error;
        }
        sd_write_udword(w, (uint32_t)c_pos);
        if(sd_writer_seek_start(w, (uint32_t)c_pos) < 0) {
            goto error;
        }
    }

    // Write logos
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        if(trn->locales[i] != NULL) {
            sd_sprite_save(w, trn->locales[i]->logo);
        } else {
            sd_sprite s;
            sd_sprite_create(&s);
            sd_sprite_save(w, &s);
            sd_sprite_free(&s);
        }
    }

    // Save 40 colors
    palette_save_range(w, &trn->pal, 128, 40);

    // Pic filename
    sd_write_variable_str(w, trn->pic_file);

    // Write tournament descriptions
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        if(trn->locales[i] != NULL) {
            sd_write_variable_str(w, trn->locales[i]->title);
            sd_write_variable_str(w, trn->locales[i]->description);
        } else {
            sd_write_variable_str(w, "");
            sd_write_variable_str(w, "");
        }
    }

    // Let's write our current offset to the victory text offset position
    long offset = sd_writer_pos(w);
    if(offset < 0) {
        goto error;
    }
    if(sd_writer_seek_start(w, 4) < 0) {
        goto error;
    }
    sd_write_dword(w, (uint32_t)offset);
    if(sd_writer_seek_start(w, offset) < 0) {
        goto error;
    }

    // Write texts
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        for(int har = 0; har < 11; har++) {
            for(int page = 0; page < 10; page++) {
                if(trn->locales[i] != NULL) {
                    sd_write_variable_str(w, trn->locales[i]->end_texts[har][page]);
                } else {
                    sd_write_variable_str(w, "");
                }
            }
        }
    }

    if(sd_writer_errno(w)) {
        goto error;
    }

    // All done. Flush and close.
    sd_writer_close(w);
    return SD_SUCCESS;

error:
    remove(filename);
    sd_writer_close(w);
    return SD_FILE_WRITE_ERROR;
}

void sd_tournament_free(sd_tournament_file *trn) {
    if(trn == NULL) {
        return;
    }
    free_locales(trn);
    free_enemies(trn);
    if(trn->pic_file != NULL) {
        omf_free(trn->pic_file);
    }
}
