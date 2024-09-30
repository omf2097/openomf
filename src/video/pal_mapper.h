#ifndef PAL_MAPPER_H
#define PAL_MAPPER_H

#include <stdbool.h>

#include "video/color.h"
#include "video/vga_palette.h"

void pal_mapper_init(void);
void pal_mapper_close(void);

void pal_mapper_clear(void);
void pal_mapper_reset(const vga_palette *base_palette);

void pal_mapper_push_range(const vga_palette *pal, int start_index, int end_index);
void pal_mapper_push_all(const vga_palette *pal);

void pal_mapper_pop_one(void);
void pal_mapper_pop_all(void);
void pal_mapper_pop_num(int i);

bool pal_mapper_is_dirty(void);
void pal_mapper_set_dirty(bool is_dirty);
void pal_mapper_flatten(char *palette);

#endif // PAL_MAPPER_H
