/**
 * @file vga_extended_palette.c
 * @brief Extended palette population and remap table builders for 1024-color mode
 */

#include "video/vga_extended_palette.h"

#ifdef USE_EXTENDED_PALETTE

#include "resources/af.h"
#include "resources/animation.h"
#include "resources/sprite.h"
#include "utils/array.h"
#include "utils/miscmath.h"
#include "utils/oklab.h"
#include "video/surface.h"
#include "video/vga_state.h"
#include "video/vga_common_colors.h"
#include <string.h>

/* ===== Public functions ===== */

void vga_extended_palette_generate_detail(vga_palette *palette, vga_index detail_start,
                                          const vga_color channels[VGA_HAR_CHANNELS][VGA_HAR_BASE_HUES]) {
    /* 16 detail colors: 6 for channel 0, 5 for channel 1, 5 for channel 2.
     * All blending done in oklab space for perceptual uniformity. */
    int idx = 0;

    for(int ch = 0; ch < VGA_HAR_CHANNELS; ch++) {
        /* Midtone in oklab: average of hues 6-10 */
        oklab_color mid = {0, 0, 0};
        for(int i = 6; i < 11; i++) {
            oklab_color c = rgb_to_oklab(channels[ch][i].r, channels[ch][i].g, channels[ch][i].b);
            mid.L += c.L; mid.a += c.a; mid.b += c.b;
        }
        mid.L /= 5; mid.a /= 5; mid.b /= 5;

        /* Desaturated: move toward L axis (a=0, b=0) at 50% */
        {
            oklab_color desat = {mid.L, mid.a * 0.5, mid.b * 0.5};
            uint8_t r, g, b;
            oklab_to_rgb(desat, &r, &g, &b);
            palette->colors[detail_start + idx] = (vga_color){r, g, b};
            idx++;
        }

        /* Heat: blend toward warm orange in oklab */
        {
            oklab_color heat = rgb_to_oklab(255, 140, 0);
            oklab_color blend = {mid.L * 0.7 + heat.L * 0.3,
                                 mid.a * 0.7 + heat.a * 0.3,
                                 mid.b * 0.7 + heat.b * 0.3};
            uint8_t r, g, b;
            oklab_to_rgb(blend, &r, &g, &b);
            palette->colors[detail_start + idx] = (vga_color){r, g, b};
            idx++;
        }

        /* Cold: blend toward blue in oklab */
        {
            oklab_color cold = rgb_to_oklab(40, 80, 200);
            oklab_color blend = {mid.L * 0.7 + cold.L * 0.3,
                                 mid.a * 0.7 + cold.a * 0.3,
                                 mid.b * 0.7 + cold.b * 0.3};
            uint8_t r, g, b;
            oklab_to_rgb(blend, &r, &g, &b);
            palette->colors[detail_start + idx] = (vga_color){r, g, b};
            idx++;
        }

        /* Accent from other channels (oklab midpoint) */
        for(int other_offset = 1; other_offset <= 2; other_offset++) {
            int other = (ch + other_offset) % VGA_HAR_CHANNELS;
            oklab_color other_mid = {0, 0, 0};
            for(int i = 6; i < 11; i++) {
                oklab_color c = rgb_to_oklab(channels[other][i].r, channels[other][i].g, channels[other][i].b);
                other_mid.L += c.L; other_mid.a += c.a; other_mid.b += c.b;
            }
            other_mid.L /= 5; other_mid.a /= 5; other_mid.b /= 5;
            oklab_color blend = {(mid.L + other_mid.L) * 0.5,
                                 (mid.a + other_mid.a) * 0.5,
                                 (mid.b + other_mid.b) * 0.5};
            uint8_t r, g, b;
            oklab_to_rgb(blend, &r, &g, &b);
            palette->colors[detail_start + idx] = (vga_color){r, g, b};
            idx++;
        }

        /* Channel 0 gets one extra: dark accent (reduce L by 40%) */
        if(ch == 0) {
            oklab_color dark = {mid.L * 0.6, mid.a * 0.6, mid.b * 0.6};
            uint8_t r, g, b;
            oklab_to_rgb(dark, &r, &g, &b);
            palette->colors[detail_start + idx] = (vga_color){r, g, b};
            idx++;
        }
    }
}

/* ===== Remap Table Builders ===== */

void vga_extended_palette_build_har_remap(vga_remap_table *remap, int player_index) {
    /* Start with identity */
    for(int i = 0; i < VGA_PALETTE_SIZE; i++) {
        remap->data[i] = i;
    }

    vga_index player_ext_start = (player_index == 0) ? VGA_EXT_P1_HAR_START : VGA_EXT_P2_HAR_START;
    vga_index detail_start = (player_index == 0) ? VGA_EXT_P1_DETAIL_START : VGA_EXT_P2_DETAIL_START;

    /* P2 zone (0x30-0x5F, 48 indices) → player extended (first 48 of 96) */
    for(int i = 0x30; i <= 0x5F; i++) {
        remap->data[i] = player_ext_start + (i - 0x30);
    }

    /* BG zone first half (0x60-0x7F, 32 indices) → player extended (next 32 of 96) */
    for(int i = 0x60; i <= 0x7F; i++) {
        remap->data[i] = player_ext_start + 48 + (i - 0x60);
    }

    /* BG zone next 16 (0x80-0x8F) → detail colors */
    for(int i = 0x80; i <= 0x8F; i++) {
        remap->data[i] = detail_start + (i - 0x80);
    }

    /* Remaining BG (0x90-0x9B, 12 indices) → extended common */
    for(int i = 0x90; i <= 0x9B; i++) {
        remap->data[i] = VGA_EXT_COMMON_START + (i - 0x90);
    }

    /* 0x9C-0x9F: identity (spare) */

    /* UI zone (0xF4-0xFF, 12 indices) → extended common */
    for(int i = 0xF4; i <= 0xFF; i++) {
        remap->data[i] = VGA_EXT_COMMON_START + (i - 0xF4);
    }
}

void vga_extended_palette_build_scene_remap(vga_remap_table *remap) {
    for(int i = 0; i < VGA_PALETTE_SIZE; i++) {
        remap->data[i] = i;
    }

    /* HAR zone (0x01-0x5F, 95 indices) → scene extended (skip 0 = transparent) */
    for(int i = 0x01; i <= 0x5F; i++) {
        remap->data[i] = VGA_EXT_SCENE_START + i;
    }

    /* UI zone (0xF4-0xFF, 12 indices) → extended common */
    for(int i = 0xF4; i <= 0xFF; i++) {
        remap->data[i] = VGA_EXT_COMMON_START + (i - 0xF4);
    }
}

void vga_extended_palette_build_portrait_remap(vga_remap_table *remap, int slot_index) {
    for(int i = 0; i < VGA_PALETTE_SIZE; i++) {
        remap->data[i] = i;
    }

    vga_index slot_start = VGA_EXT_SLOT1_START + slot_index * VGA_EXT_SLOT_SIZE;

    /* Expanded common (0x01-0x5F, 95 indices) → expanded common zone.
     * Per the extended palette plan, portraits address 84 base common +
     * 95 expanded common + 64 slot + 12 ext common = 256 total.
     * Expanded common colors are hardcoded in vga_state_init — no
     * load_mod_colors copy needed for these indices. */
    for(int i = 0x01; i <= 0x5F; i++) {
        remap->data[i] = VGA_EXT_EXPANDED_COMMON_START + i;
    }

    /* Per-slot custom (0x60-0x9F, 64 indices) → portrait slot zone
     * paltool packs 64 custom colors here. Each portrait gets its own
     * 64-color slot for custom colors. */
    for(int i = 0x60; i <= 0x9F; i++) {
        remap->data[i] = slot_start + (i - 0x60);
    }

    /* BK common (0xA0-0xF3, 84 indices) → identity (base palette) */

    /* Extended common (0xF4-0xFF, 12 indices) → extended common zone */
    for(int i = 0xF4; i <= 0xFF; i++) {
        remap->data[i] = VGA_EXT_COMMON_START + (i - 0xF4);
    }
}

void vga_extended_palette_build_common_sprite_remap(vga_remap_table *remap, int slot_index) {
    vga_extended_palette_build_portrait_remap(remap, slot_index);
}

void vga_extended_palette_build_scene_common_remap(vga_remap_table *remap) {
    /* SCENE_COMMON shares slot with PORTRAIT_3 — they're never used together.
     * Redirects HAR zone to expanded common, UI zone to extended common. */
    for(int i = 0; i < VGA_PALETTE_SIZE; i++) {
        remap->data[i] = i;
    }

    /* HAR zone (0x01-0x5F, 95 indices) → expanded common (skip 0 = transparent) */
    for(int i = 0x01; i <= 0x5F; i++) {
        remap->data[i] = VGA_EXT_EXPANDED_COMMON_START + i;
    }

    /* UI zone (0xF4-0xFF, 12 indices) → extended common */
    for(int i = 0xF4; i <= 0xFF; i++) {
        remap->data[i] = VGA_EXT_COMMON_START + (i - 0xF4);
    }
}

/* Global sprite-type remap tables, built once at scene init */
#define SPRITE_REMAP_TYPE_COUNT 8
static vga_remap_table sprite_remaps[SPRITE_REMAP_TYPE_COUNT];
static bool sprite_remaps_initialized = false;

void vga_extended_palette_init_sprite_remaps(void) {
    vga_extended_palette_build_har_remap(&sprite_remaps[0], 0); // HAR_P1
    vga_extended_palette_build_har_remap(&sprite_remaps[1], 1); // HAR_P2
    vga_extended_palette_build_scene_remap(&sprite_remaps[2]);   // SCENE
    vga_extended_palette_build_portrait_remap(&sprite_remaps[3], 0); // PORTRAIT_1
    vga_extended_palette_build_portrait_remap(&sprite_remaps[4], 1); // PORTRAIT_2
    vga_extended_palette_build_scene_common_remap(&sprite_remaps[5]); // PORTRAIT_3 / SCENE_COMMON
    vga_extended_palette_build_portrait_remap(&sprite_remaps[6], 3); // PORTRAIT_4
    vga_extended_palette_build_portrait_remap(&sprite_remaps[7], 4); // PORTRAIT_5
    sprite_remaps_initialized = true;
}

const vga_remap_table *vga_extended_palette_get_sprite_remap(int sprite_type) {
    if(!sprite_remaps_initialized) {
        vga_extended_palette_init_sprite_remaps();
    }
    if(sprite_type < 0 || sprite_type >= SPRITE_REMAP_TYPE_COUNT) {
        return NULL;
    }
    return &sprite_remaps[sprite_type];
}

void vga_extended_palette_load_mod_colors(const vga_palette *mod_pal, int sprite_type) {
    /* Walk the sprite-type remap table. For each index that remaps
     * to an extended palette target (>255), copy the mod palette
     * color at that index to the target position in the active palette.
     * Skip static zones (extended common, expanded common) — those
     * are hardcoded and must not be overwritten by mod palettes. */
    const vga_remap_table *remap = vga_extended_palette_get_sprite_remap(sprite_type);
    if(remap == NULL || mod_pal == NULL) {
        return;
    }

    for(int i = 0; i < 256; i++) {
        vga_index target = remap->data[i];
        if(target >= 256) {
            // Skip static zones — these are set at init/scene-change and must not be clobbered
            if(target >= VGA_EXT_COMMON_START && target <= VGA_EXT_COMMON_END) {
                continue;
            }
            if(target >= VGA_EXT_EXPANDED_COMMON_START && target <= VGA_EXT_EXPANDED_COMMON_END) {
                continue;
            }
            vga_state_set_base_palette_index(target, &mod_pal->colors[i]);
        }
    }
}

void vga_extended_palette_set_har_sprite_remaps(af *af_data, int player_index) {
    if(af_data == NULL) {
        return;
    }
    int sprite_type = (player_index == 0) ? SPRITE_REMAP_HAR_P1 : SPRITE_REMAP_HAR_P2;
    const vga_remap_table *remap = vga_extended_palette_get_sprite_remap(sprite_type);
    if(remap == NULL) {
        return;
    }

    // Walk all moves via sparse array iterator
    iterator it;
    af_move *move = NULL;
    array_iter_begin(&af_data->moves, &it);
    foreach(it, move) {
        animation *ani = &move->ani;
        int count = animation_get_sprite_count(ani);
        for(int s = 0; s < count; s++) {
            sprite *spr = animation_get_sprite(ani, s);
            if(spr && spr->data) {
                surface_set_remap(spr->data, remap);
            }
        }
    }
}

#endif /* USE_EXTENDED_PALETTE */
