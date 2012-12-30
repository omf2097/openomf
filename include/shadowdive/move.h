#ifndef MOVE_H
#define MOVE_H

#include "shadowdive/animation.h"

typedef struct sd_writer_t sd_writer;
typedef struct sd_reader_t sd_reader;

typedef struct sd_move_t {
    sd_animation *animation;
    char unknown[21];
    char move_string[21];
    char *footer_string;
} sd_move;

sd_move* sd_move_create();
void sd_move_delete(sd_move *move);
int sd_move_load(sd_reader *reader, sd_move *move);
void sd_move_save(sd_writer *writer, sd_move *move);

#endif // _ANIMATION_H
