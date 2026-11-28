#include "video/software/compositor.h"

#include "video/enums.h"
#include "video/video.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static int clamp_index(int index) {
    if(index < 0) {
        return 0;
    }
    return index >= VGA_PALETTE_SIZE ? VGA_PALETTE_SIZE - 1 : index;
}

bool software_compositor_init(software_compositor *ctx) {
    if(!ctx) {
        return false;
    }
    memset(ctx, 0, sizeof(*ctx));
    ctx->indexed = calloc((size_t)NATIVE_W * NATIVE_H, sizeof(software_pixel));
    ctx->rgba = calloc((size_t)NATIVE_W * NATIVE_H, sizeof(uint32_t));
    if(!ctx->indexed || !ctx->rgba) {
        software_compositor_close(ctx);
        return false;
    }
    return true;
}

void software_compositor_close(software_compositor *ctx) {
    if(ctx) {
        free(ctx->indexed);
        free(ctx->rgba);
        ctx->indexed = NULL;
        ctx->rgba = NULL;
    }
}

void software_compositor_prepare(software_compositor *ctx, unsigned framebuffer_options) {
    ctx->framebuffer_options = framebuffer_options;
    memset(ctx->indexed, 0, (size_t)NATIVE_W * NATIVE_H * sizeof(software_pixel));
}

void software_compositor_set_palette(software_compositor *ctx, const vga_palette *palette) {
    memcpy(&ctx->palette, palette, sizeof(*palette));
}

void software_compositor_set_remaps(software_compositor *ctx, const vga_remap_tables *remaps) {
    memcpy(&ctx->remaps, remaps, sizeof(*remaps));
}

static int remap_index(const software_compositor *ctx, int index, int row) {
    if(row < 0 || row >= VGA_REMAP_COUNT) {
        return clamp_index(index);
    }
    return ctx->remaps.tables[row].data[clamp_index(index)];
}

void software_compositor_draw(software_compositor *ctx, const surface *src, const irect *dst, int remap_offset,
                              int remap_rounds, int palette_offset, int palette_limit, int opacity, unsigned flip,
                              unsigned options) {
    if(!ctx || !ctx->indexed || !src || !src->data || !dst || src->w <= 0 || src->h <= 0 || dst->w <= 0 ||
       dst->h <= 0 || opacity <= 0) {
        return;
    }

    const int64_t right = (int64_t)dst->x + dst->w;
    const int64_t bottom = (int64_t)dst->y + dst->h;
    if(right <= 0 || bottom <= 0 || dst->x >= NATIVE_W || dst->y >= NATIVE_H) {
        return;
    }
    const int x0 = dst->x < 0 ? 0 : dst->x;
    const int y0 = dst->y < 0 ? 0 : dst->y;
    const int x1 = right > NATIVE_W ? NATIVE_W : (int)right;
    const int y1 = bottom > NATIVE_H ? NATIVE_H : (int)bottom;
    if(x0 >= x1 || y0 >= y1) {
        return;
    }

    for(int y = y0; y < y1; y++) {
        /* Sample at the destination pixel center, including when clipped. */
        const int64_t sy_num = (2 * ((int64_t)y - dst->y) + 1) * src->h;
        const int sy_center = (int)(sy_num / (2 * dst->h));
        int sy = sy_center;
        if(flip & FLIP_VERTICAL) {
            sy = src->h - sy - 1;
        }
        const float noise_phase =
            opacity < 255 ? tanf(10.0f * 1.6180339887498948482f * ((float)NATIVE_H - y - 0.5f)) : 0.0f;
        for(int x = x0; x < x1; x++) {
            if(opacity < 255) {
                const float phase = noise_phase + (107.0f * ((float)x + 0.5f)) / 256.0f;
                if(phase - floorf(phase) > (float)opacity / 255.0f) {
                    continue;
                }
            }
            const int64_t sx_num = (2 * ((int64_t)x - dst->x) + 1) * src->w;
            const int sx_center = (int)(sx_num / (2 * dst->w));
            int sx = sx_center;
            if(flip & FLIP_HORIZONTAL) {
                sx = src->w - sx - 1;
            }

            int idx = src->data[(size_t)sy * src->w + sx];
            if(options & SPRITE_SHADOW) {
                int coverage = 0;
                for(int i = -1; i <= 2; i++) {
                    const int sample_y = sy + i;
                    if(sample_y >= 0 && sample_y < src->h &&
                       src->data[(size_t)sample_y * src->w + sx] != src->transparent) {
                        coverage++;
                    }
                }
                if(!coverage) {
                    continue;
                }
                idx = coverage;
            } else if(idx == src->transparent) {
                continue;
            }

            if(idx <= palette_limit) {
                idx = clamp_index(idx + palette_offset);
                if(idx > palette_limit) {
                    idx = palette_limit;
                }
            }
            if((options & SPRITE_REMAP) && remap_offset >= 0) {
                idx = remap_index(ctx, idx, remap_offset);
            }

            software_pixel *pixel = &ctx->indexed[(size_t)y * NATIVE_W + x];
            if(options & SPRITE_TRANSLUCENT) {
                if(idx < 0x80) {
                    idx = (idx & 0xf0) + ((idx & 0x0f) >> 1);
                }
                pixel->translucent = (uint16_t)(clamp_index(idx) + 1);
            } else if((options & SPRITE_SHADOW) || remap_rounds > 0) {
                const int encoded = remap_offset + remap_rounds * VGA_REMAP_COUNT + idx;
                const uint8_t value = (uint8_t)(encoded < 0 ? 0 : encoded > 255 ? 255 : encoded);
                if(options & SPRITE_SHADOW) {
                    if(value > pixel->remap) {
                        pixel->remap = value;
                    }
                } else {
                    pixel->remap = value;
                }
            } else if(options & SPRITE_INDEX_ADD) {
                const int additive = idx * 60;
                pixel->additive = (uint16_t)(additive > 1023 ? 1023 : additive);
            } else {
                pixel->index = (uint16_t)clamp_index(idx);
                pixel->translucent = 0;
                pixel->additive = 0;
                pixel->remap = 0;
            }
        }
    }
}

const uint32_t *software_compositor_resolve(software_compositor *ctx) {
    for(int i = 0; i < NATIVE_W * NATIVE_H; i++) {
        const software_pixel *pixel = &ctx->indexed[i];
        int idx = clamp_index(pixel->index + pixel->additive);
        if(!(ctx->framebuffer_options & FBUFOPT_CREDITS)) {
            if(pixel->translucent) {
                const int shade = pixel->translucent - 1;
                if(shade < 0x80) {
                    const vga_color base = ctx->palette.colors[idx];
                    const int brightness = (base.r + base.g + base.b) / 3;
                    idx = clamp_index(shade + (brightness >> 5));
                } else {
                    idx = clamp_index(shade);
                }
            }
            const int rounds = pixel->remap / VGA_REMAP_COUNT;
            const int row = pixel->remap % VGA_REMAP_COUNT;
            for(int round = 0; round < rounds; round++) {
                idx = clamp_index(remap_index(ctx, idx, row));
            }
        }

        if((ctx->framebuffer_options & FBUFOPT_CREDITS) && pixel->index + pixel->additive >= VGA_PALETTE_SIZE) {
            ctx->rgba[i] = 0xff000000u;
        } else {
            const vga_color color = ctx->palette.colors[idx];
            ctx->rgba[i] = 0xff000000u | ((uint32_t)color.b << 16) | ((uint32_t)color.g << 8) | color.r;
        }
    }
    return ctx->rgba;
}

void software_compositor_capture_area(const software_compositor *ctx, const irect *area, surface *dst) {
    if(!ctx || !ctx->indexed || !area || !dst || area->w <= 0 || area->h <= 0) {
        return;
    }
    surface_create(dst, area->w, area->h);
    dst->transparent = -1;
    for(int y = 0; y < area->h; y++) {
        const int64_t sy = (int64_t)area->y + y;
        for(int x = 0; x < area->w; x++) {
            const int64_t sx = (int64_t)area->x + x;
            if(sx >= 0 && sx < NATIVE_W && sy >= 0 && sy < NATIVE_H) {
                dst->data[(size_t)y * dst->w + x] = (vga_pixel)clamp_index(ctx->indexed[sy * NATIVE_W + sx].index);
            }
        }
    }
}
