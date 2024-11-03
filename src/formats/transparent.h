/*! \file
 * \brief Transparent VGA pixel values
 * \details Describes the transparancy index value for various engine components.
 * \copyright MIT license.
 * \date 2024
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef TRANSPARENT_H
#define TRANSPARENT_H

// Default's.
// Please do not use the default definitions in the code directly. Define a new value below instead.
// This way all different transparency values are stored in one place.
#define DEFAULT_TRANSPARENT_INDEX 0
#define DEFAULT_NOT_TRANSPARENT -1

// All sprites have holes, these holes need to be transparent.
#define SPRITE_TRANSPARENT_INDEX DEFAULT_TRANSPARENT_INDEX

// Backgrounds have no transparent pixels and need all pixels to be rendered.
#define BACKGROUND_TRANSPARENT_INDEX DEFAULT_NOT_TRANSPARENT

// Force an opaque menu for now.
#define MENU_TRANSPARENT_INDEX DEFAULT_NOT_TRANSPARENT

// Portraits have a transparancy index that needs to offset past other items in the global palette.
// TODO: Should really become default.
#define PORTRAIT_TRANSPARENT_INDEX 0xD0

// Fonts need a transparent index that does not conflict with the lower 8 colors on the N64 otherwise there will be a
// combinatorial explosion.
#define FONT_TRANSPARENT_INDEX DEFAULT_TRANSPARENT_INDEX
#define PCX_FONT_TRANSPARENT_INDEX DEFAULT_TRANSPARENT_INDEX

// Gauges in the player select have an opaque background and are not transparent.
#define GAUGE_TRANSPARENT_INDEX DEFAULT_NOT_TRANSPARENT

#endif
