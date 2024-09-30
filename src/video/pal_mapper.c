#include <string.h>

#include "utils/log.h"
#include "utils/vector.h"
#include "video/pal_mapper.h"

typedef struct {
    vector layers;
    bool is_dirty;
} state;

typedef struct {
    int range_start;
    int range_end;
    unsigned char *buffer;
} layer;

static state g_state;

void pal_mapper_init(void) {
    memset(&g_state, 0, sizeof(state));
    vector_create(&g_state.layers, sizeof(layer));
    g_state.is_dirty = false;
    DEBUG("Palette mapper created!");
}

void pal_mapper_close(void) {
    vector_free(&g_state.layers);
    memset(&g_state, 0, sizeof(state));
    DEBUG("Palette mapper freed!");
}

void pal_mapper_clear(void);
void pal_mapper_reset(const vga_palette *base_palette);

void pal_mapper_push_range(const vga_palette *pal, int start_index, int end_index);
void pal_mapper_push_all(const vga_palette *pal);

void pal_mapper_pop_one(void);
void pal_mapper_pop_all(void);
void pal_mapper_pop_num(int i);

bool pal_mapper_is_dirty(void) {
    return g_state.is_dirty;
}

void pal_mapper_set_dirty(bool is_dirty) {
    g_state.is_dirty = is_dirty;
}

void pal_mapper_flatten(char *palette) {
}
