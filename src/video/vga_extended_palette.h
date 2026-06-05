/**
 * @file vga_extended_palette.h
 * @brief Extended palette layout constants for 1024-color mode
 *
 * When USE_EXTENDED_PALETTE is defined, the VGA palette expands from 256
 * to 1024 colors. This header defines the layout zones and remap targets
 * per the extended palette plan.
 *
 * Base palette (0x000-0x0FF) is unchanged from the original 256-color layout.
 * Extended zones (0x100-0x3FF) are populated at palette load time based on
 * scene type and player selections.
 *
 * Additional documentation is available at https://www.omf2097.com/wiki/doku.php?id=openomf:palettes
 */

#ifndef VGA_EXTENDED_PALETTE_H
#define VGA_EXTENDED_PALETTE_H

#include "resources/af.h"
#include "video/vga_palette.h"
#include "video/vga_remap.h"

#ifdef USE_EXTENDED_PALETTE

/* Base Palette Zones (0x000-0x0FF) - not changed */

#define VGA_BASE_P1_HAR_START 0x01
#define VGA_BASE_P1_HAR_END 0x2F   /* 47 colors, 3ch x 16 hues */
#define VGA_BASE_P1_BLACK 0x00     /* Always black (transparency) */
#define VGA_BASE_P1_SEPARATOR 0x30 /* Always black */

#define VGA_BASE_P2_HAR_START 0x31
#define VGA_BASE_P2_HAR_END 0x5F /* 47 colors, 3ch x 16 hues */

#define VGA_BASE_BG_START 0x60
#define VGA_BASE_BG_END 0x9F /* 64 background/arena colors */

#define VGA_BASE_COMMON_START 0xA0
#define VGA_BASE_COMMON_END 0xF3 /* 84 common colors */

#define VGA_BASE_RESERVED_START 0xF4
#define VGA_BASE_RESERVED_END 0xF9 /* 6 reserved/cursor */
#define VGA_BASE_MENU_START 0xFA
#define VGA_BASE_MENU_END 0xFF /* 6 menu colors */

/* Extended common: 12 colors visible to ALL sprite types.
 * Remapped from UI indices 0xF4-0x0FF.
 * see vga_ext_common in vga_common_colors.h
 */
#define VGA_EXT_COMMON_START 0x100
#define VGA_EXT_COMMON_END 0x10B /* 12 colors */

/* P1 HAR player extended: 32 new interpolated hues per channel (96 total).
 * Combined with 48 originals in base = 144 P1 player colors.
 * Remapped from P2 zone (0x30-0x5F). */
#define VGA_EXT_P1_HAR_START 0x10C /* 96 colors: 3ch x 32 hues */

/* P1 HAR detail: 16 auto-generated colors derived from pilot base colors.
 * Desaturated, heat, cold, accent operations. */
#define VGA_EXT_P1_DETAIL_START 0x16C /* 16 colors */

/* P2 HAR player extended: same structure as P1, for player 2. */
#define VGA_EXT_P2_HAR_START 0x17C /* 96 colors: 3ch x 32 hues */

/* P2 HAR detail: same structure as P1 detail, for player 2. */
#define VGA_EXT_P2_DETAIL_START 0x1DC /* 16 colors */

/* Scene extended: 96 colors for backgrounds, overlays, hazards.
 * Populated per scene from BK data. Remapped from HAR color indices */
#define VGA_EXT_SCENE_START 0x1EC /* 96 colors */

/* Expanded common: 96 colors shared by portraits and common arena/fighter sprites.
 * Remapped from HAR zone indices (0x00-0x5F).
 * Contains slide midpoints and new color families.
 * See vga_ext_expanded_common in vga_common_colors.h
 * */
#define VGA_EXT_EXPANDED_COMMON_START 0x24C
#define VGA_EXT_EXPANDED_COMMON_END 0x2AB /* 96 colors */

/* Portrait/slot custom: 5 slots x 64 colors each.
 * Slot 1 is always P1 portrait (stays loaded across tournament screens).
 * The other slots can be variously repurposed depending on need.
 */
#define VGA_EXT_SLOT1_START 0x2AC
#define VGA_EXT_SLOT1_END 0x2EB /* 64 colors */
#define VGA_EXT_SLOT2_START 0x2EC
#define VGA_EXT_SLOT2_END 0x32B /* 64 colors */
#define VGA_EXT_SLOT3_START 0x32C
#define VGA_EXT_SLOT3_END 0x36B /* 64 colors */
#define VGA_EXT_SLOT4_START 0x36C
#define VGA_EXT_SLOT4_END 0x3AB /* 64 colors */
#define VGA_EXT_SLOT5_START 0x3AC
#define VGA_EXT_SLOT5_END 0x3EB /* 64 colors */

/* Spare entries, this is where we store the colors THEY don't want you to see! */
#define VGA_EXT_SPARE_START 0x3EC
#define VGA_EXT_SPARE_END 0x3FF /* 20 colors */

/* ===== Zone sizes ===== */

#define VGA_EXT_COMMON_SIZE 12
#define VGA_EXT_HAR_PLAYER_SIZE 96 /* per player */
#define VGA_EXT_HAR_DETAIL_SIZE 16 /* per player */
#define VGA_EXT_SCENE_SIZE 96
#define VGA_EXT_EXPANDED_COMMON_SIZE 96
#define VGA_EXT_SLOT_SIZE 64
#define VGA_EXT_SLOT_COUNT 5

/* ===== Per-channel constants ===== */

#define VGA_HAR_CHANNELS 3
#define VGA_HAR_BASE_HUES 16     /* per channel, in base palette */
#define VGA_HAR_EXTENDED_HUES 32 /* per channel, new interpolated */
#define VGA_HAR_TOTAL_HUES 48    /* per channel, base + extended */

/* ===== Remap target indices ===== */
/* These define which extended zone each illegal index range remaps to,
 * per sprite type. Used when building remap tables. */

/* HAR sprite remap targets (124 illegal indices):
 *   0x30-0x5F (48) → P1: ext P1 player, P2: ext P2 player
 *   0x60-0x7F (32) → ext player (part of 96 for player extended)
 *   0x80-0x9F (32) → ext detail (16) + ext common (12) + 4 spare from BG zone
 *   0xF4-0xFF (12) → ext common
 */

/* Scene background/sprite remap targets (108 illegal indices):
 *   0x00-0x5F (96) → scene extended
 *   0xF4-0xFF (12) → ext common
 */

/* Portrait remap targets (per extended_palette_plan3.dokuwiki):
 *   0x00-0x5F (96) → expanded common (portrait base colors)
 *   0x60-0x9F (64) → slot zone (per-slot custom, 64 colors)
 *   0xA0-0xF3 (84) → identity (BK common, base palette)
 *   0xF4-0xFF (12) → ext common
 * Total: 84 base common + 96 expanded common + 64 slot + 12 ext common = 256
 */

/* Common arena/fighter sprite remap targets (172 illegal indices):
 *   0x00-0x5F (96) → expanded common
 *   0x60-0x9F (64) → slot custom (slot 3 in arena)
 *   0xF4-0xFF (12) → ext common
 */

/* ===== Function declarations ===== */

/**
 * @brief Generate HAR detail colors from pilot base colors
 * @param palette Palette to write into
 * @param detail_start Start index in extended palette for detail block
 * @param channels Array of 3 x 16 vga_color values (all 3 channels)
 */
void vga_extended_palette_generate_detail(vga_palette *palette, vga_index detail_start,
                                          const vga_color channels[VGA_HAR_CHANNELS][VGA_HAR_BASE_HUES]);

/**
 * @brief Build remap table for a HAR sprite (per player)
 * @param remap Remap table to populate (1024 entries)
 * @param player_index 0 for P1, 1 for P2
 */
void vga_extended_palette_build_har_remap(vga_remap_table *remap, int player_index);

/**
 * @brief Build remap table for scene background/sprites
 * @param remap Remap table to populate
 */
void vga_extended_palette_build_scene_remap(vga_remap_table *remap);

/**
 * @brief Build remap table for a portrait
 * @param remap Remap table to populate
 * @param slot_index Portrait slot (0-4)
 */
void vga_extended_palette_build_portrait_remap(vga_remap_table *remap, int slot_index);

/**
 * @brief Build remap table for common arena/fighter sprites
 * @param remap Remap table to populate
 * @param slot_index Custom slot index (slot 3 = index 2 for arena)
 */
void vga_extended_palette_build_common_sprite_remap(vga_remap_table *remap, int slot_index);

/**
 * Sprite-type remap indices. These are separate from BK effect remaps
 * (which use rows 0-18 in vga_remap_tables). Sprite-type remaps redirect
 * illegal palette indices into extended zones at upload time.
 */
enum
{
    SPRITE_REMAP_HAR_P1 = 0,
    SPRITE_REMAP_HAR_P2 = 1,
    SPRITE_REMAP_SCENE = 2,
    SPRITE_REMAP_PORTRAIT_1 = 3,
    SPRITE_REMAP_PORTRAIT_2 = 4,
    SPRITE_REMAP_PORTRAIT_3 = 5,
    SPRITE_REMAP_SCENE_COMMON = SPRITE_REMAP_PORTRAIT_3, // same slot — never used together
    SPRITE_REMAP_PORTRAIT_4 = 6,
    SPRITE_REMAP_PORTRAIT_5 = 7,
    SPRITE_REMAP_TYPE_COUNT = 8,
};

/**
 * @brief Get the global sprite-type remap table for a given type
 * @param sprite_type One of SPRITE_REMAP_HAR_P1, etc.
 * @return Pointer to the global remap table (built once at scene init)
 */
const vga_remap_table *vga_extended_palette_get_sprite_remap(int sprite_type);

/**
 * @brief Initialize sprite-type remap tables (called once at scene init)
 */
void vga_extended_palette_init_sprite_remaps(void);

/**
 * @brief Load mod sprite palette colors into extended palette zones
 *
 * Walks the sprite-type remap table. For each index that remaps to an
 * extended palette target (>255), copies the mod palette color at that
 * index to the target position in the active palette.
 *
 * @param mod_pal Mod sprite palette (256 colors)
 * @param sprite_type 0=HAR_P1, 1=HAR_P2, 2=SCENE, 3=PORTRAIT
 */
void vga_extended_palette_load_mod_colors(const vga_palette *mod_pal, int sprite_type);

/**
 * @brief Update all HAR sprite surfaces to use the correct player remap
 *
 * Walks all sprites in the AF data and sets the surface remap to the
 * appropriate player sprite-type remap table (P1 or P2). This should be
 * called after scene_load_har for each player.
 *
 * @param af_data The AF data structure containing all moves/animations
 * @param player_index 0 for P1, 1 for P2
 */
void vga_extended_palette_set_har_sprite_remaps(af *af_data, int player_index);

#endif /* USE_EXTENDED_PALETTE */
#endif /* VGA_EXTENDED_PALETTE_H */
