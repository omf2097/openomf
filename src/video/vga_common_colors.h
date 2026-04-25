/**
 * @file vga_common_colors.h
 * @brief Extended and expanded common color tables for 1024-color palette mode
 *
 * These static const arrays define the shared color tables used by:
 * - vga_state_init (palette initialization)
 * - vga_extended_palette (remap and detail generation)
 * - paltool (reference palette matching)
 *
 * All three consumers must use the same data. This header is the single
 * source of truth.
 */

#ifndef VGA_COMMON_COLORS_H
#define VGA_COMMON_COLORS_H

#include "video/vga_palette.h"

#ifdef USE_EXTENDED_PALETTE

/* Extended common: 12 colors visible to ALL sprite types.
 * Brown, Pink/Magenta, Forest Green (4 each).
 * Written to palette at VGA_EXT_COMMON_START (0x100). */
static const vga_color vga_ext_common[12] = {
    /* Brown (4) */
    {40, 28, 16},   /* dark brown */
    {72, 48, 28},   /* brown */
    {105, 72, 44},  /* medium brown */
    {140, 97, 60},  /* light brown */
    /* Pink/Magenta (4) */
    {93, 0, 48},    /* dark magenta */
    {157, 24, 85},  /* magenta */
    {214, 60, 133}, /* pink */
    {255, 121, 186},/* light pink */
    /* Forest Green (4) */
    {8, 40, 16},    /* dark forest */
    {24, 76, 32},   /* forest green */
    {48, 113, 56},  /* medium forest */
    {80, 153, 85},  /* light forest */
};

/* Expanded common: 96 colors shared by portraits and common arena/fighter sprites.
 * 56 slide midpoints + 40 new color families.
 * Written to palette at VGA_EXT_EXPANDED_COMMON_START (0x24C). */
static const vga_color vga_ext_expanded_common[96] = {
    /* Green midpoints (0x24C-0x252) */
    {0, 93, 0}, {0, 125, 0}, {0, 157, 0}, {0, 190, 0}, {0, 222, 0}, {0, 239, 0}, {0, 206, 0},
    /* Royal Blue midpoints (0x253-0x259) */
    {0, 0, 93}, {0, 0, 125}, {0, 0, 157}, {0, 0, 190}, {0, 0, 222}, {0, 0, 239}, {0, 0, 206},
    /* Red midpoints (0x25A-0x260) */
    {93, 0, 0}, {125, 0, 0}, {157, 0, 0}, {190, 0, 0}, {222, 0, 0}, {239, 0, 0}, {206, 0, 0},
    /* Purple midpoints (0x261-0x267) */
    {75, 0, 93}, {101, 0, 125}, {125, 0, 157}, {147, 0, 190}, {168, 0, 222}, {186, 0, 239}, {157, 0, 206},
    /* Steel Blue midpoints (0x268-0x26E) */
    {107, 121, 125}, {129, 145, 149}, {151, 168, 172}, {172, 190, 194}, {192, 214, 218}, {212, 238, 243}, {168, 186, 190},
    /* Gold midpoints (0x26F-0x275) */
    {77, 75, 4}, {109, 103, 12}, {141, 129, 24}, {174, 156, 42}, {206, 182, 62}, {239, 208, 87}, {168, 151, 39},
    /* Light Blue midpoints (0x276-0x27C) */
    {16, 105, 125}, {36, 131, 149}, {60, 160, 172}, {89, 186, 194}, {123, 212, 218}, {164, 241, 243}, {83, 182, 190},
    /* Skin Tones midpoints (0x27D-0x283) */
    {115, 87, 56}, {143, 107, 70}, {172, 127, 85}, {200, 147, 101}, {220, 172, 125}, {238, 210, 166}, {195, 144, 98},
    /* Orange-Yellow midpoints (3) */
    {206, 89, 12}, {224, 121, 34}, {242, 153, 56},
    /* Yellow extension (6) */
    {255, 220, 0}, {255, 235, 0}, {255, 245, 0},
    {255, 250, 32}, {255, 255, 80}, {255, 255, 133},
    /* Brown lighter (4) */
    {178, 125, 80}, {210, 157, 105}, {235, 190, 140}, {252, 226, 182},
    /* Pink lighter (4) */
    {255, 153, 202}, {255, 182, 218}, {255, 210, 232}, {255, 234, 246},
    /* Forest Green lighter (4) */
    {113, 190, 117}, {153, 222, 153}, {190, 240, 190}, {222, 250, 222},
    /* Proper Yellow (4) */
    {157, 157, 0}, {198, 198, 0}, {238, 238, 0}, {255, 255, 60},
    /* Orange (2) */
    {206, 89, 12}, {242, 153, 56},
    /* Mid/Sky Blue (2) */
    {48, 85, 190}, {105, 150, 232},
    /* Emerald/Grass Green (4) */
    {16, 113, 32}, {36, 157, 52}, {64, 198, 80}, {105, 230, 117},
    /* Sage/Muted Green (2) */
    {72, 105, 68}, {117, 153, 109},
    /* Lime/Chartreuse (2) */
    {133, 190, 16}, {182, 230, 60},
    /* Olive (3) */
    {56, 72, 28}, {80, 97, 40}, {109, 125, 56},
};

#endif /* USE_EXTENDED_PALETTE */
#endif /* VGA_COMMON_COLORS_H */
