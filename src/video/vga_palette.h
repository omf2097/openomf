#ifndef VGA_PALETTE_H
#define VGA_PALETTE_H

#define VGA_REMAP_COUNT 19

typedef unsigned char vga_index;

typedef struct vga_color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} __attribute__((packed)) vga_color;

typedef struct vga_palette {
    vga_color colors[256];
} __attribute__((packed)) vga_palette;

typedef struct vga_remap_table {
    vga_index data[256];
} vga_remap_table;

typedef void (*vga_palette_transform)(vga_palette *pal, void *userdata);

typedef struct vga_state {
    vga_palette palette;
    vga_remap_table remaps[VGA_REMAP_COUNT];
    bool dirty_remaps;
    bool dirty_palette;
    unsigned char dirty_range_start;
    unsigned char dirty_range_end;
} vga_state;

void vga_state_init(void);
void vga_state_close(void);

// dirtyness chaking
bool vga_state_dirty_palette(vga_index *dirty_range_start, vga_index *dirty_range_end);
bool vga_state_dirty_remaps(void);

// These make palette dirty
void vga_set_palette(palette *src);
void vga_set_palette_range(palette *src, vga_index dst_index, vga_index src_index, vga_index count);
void vga_set_palette_index(vga_index index, vga_color color);
void vga_set_palette_indices(vga_index start, vga_index count, vga_color *src_colors);

// Apply freeform palette transform function
void vga_apply_palette_transform(vga_palette_transform transform_callback, void *userdata);

// These make remaps dirty
void vga_set_remaps(vga_remap_table **src);

#endif // VGA_PALETTE_H
