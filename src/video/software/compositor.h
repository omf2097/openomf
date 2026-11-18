#ifndef OMF_SOFTWARE_COMPOSITOR_H
#define OMF_SOFTWARE_COMPOSITOR_H

#include "utils/rect.h"
#include "video/surface.h"
#include "video/vga_remap.h"

#include <stdbool.h>
#include <stdint.h>

/* Indexed state is retained until resolve, including destination remaps. */
typedef struct software_pixel {
    uint16_t index;
    uint16_t translucent;
    uint16_t additive;
    uint8_t remap;
} software_pixel;

typedef struct software_compositor {
    software_pixel *indexed;
    uint32_t *rgba;
    vga_palette palette;
    vga_remap_tables remaps;
    unsigned framebuffer_options;
} software_compositor;

bool software_compositor_init(software_compositor *ctx);
void software_compositor_close(software_compositor *ctx);
void software_compositor_prepare(software_compositor *ctx, unsigned framebuffer_options);
void software_compositor_set_palette(software_compositor *ctx, const vga_palette *palette);
void software_compositor_set_remaps(software_compositor *ctx, const vga_remap_tables *remaps);
void software_compositor_draw(software_compositor *ctx, const surface *src, const irect *dst, int remap_offset,
                              int remap_rounds, int palette_offset, int palette_limit, int opacity, unsigned flip,
                              unsigned options);
const uint32_t *software_compositor_resolve(software_compositor *ctx);
void software_compositor_capture_area(const software_compositor *ctx, const irect *area, surface *dst);

#endif // OMF_SOFTWARE_COMPOSITOR_H
