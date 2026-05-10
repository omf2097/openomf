/**
 * @file osd.h
 * @brief This is the OnScreen Display (=OSD) system for showing arbitrary text on the screen, on top of other content.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef OSD_H
#define OSD_H

#include "video/vga_palette.h"
#include <stdarg.h>
#include <stdbool.h>

/**
 * Initialize the OSD subsystem
 */
bool osd_init(void);

/**
 * Shutdown the OSD subsystem
 */
void osd_close(void);

/**
 * Tick the OSD subsystem. This takes care of cleaning up old items.
 */
void osd_tick(void);

/**
 * Render the OSD texts and the dissolve effect.
 */
void osd_render(void);

/**
 * Print text to the OSD layer.
 * @param fmt Format string
 * @param ... Varargs
 */
void osd_print(const char *fmt, ...);

/**
 * Print text to the OSD layer using specific color.
 * @param color Text color
 * @param shadow_color Text shadow color
 * @param fmt Format string
 * @param ... Varargs
 */
void osd_print_color(vga_index color, vga_index shadow_color, const char *fmt, ...);

/**
 * Print text to the OSD layer (varargs version)
 * @param fmt Format string
 * @param ap Varargs
 */
void osd_vprint(const char *fmt, va_list ap);

/**
 * Print text to the OSD layer using specific color. (varargs version)
 * @param color Text color
 * @param shadow_color Text shadow color
 * @param fmt Format string
 * @param ap Varargs
 */
void osd_vprint_color(vga_index color, vga_index shadow_color, const char *fmt, va_list ap);

/**
 * Clear OSD layer texts completely.
 */
void osd_clear(void);

/**
 * Set default text color.
 * @param color Palette color index
 */
void osd_set_default_color(vga_index color);

/**
 * Set default text shadow color.
 * @param shadow_color Palette color index
 */
void osd_set_default_shadow_color(vga_index shadow_color);

#endif // OSD_H
