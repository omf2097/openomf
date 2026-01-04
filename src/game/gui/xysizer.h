/*! @file
 * @brief Container that positions children at absolute X/Y coordinates.
 */

#ifndef XYSIZER_H
#define XYSIZER_H

#include "game/gui/component.h"
#include "video/surface.h"

/** @brief Create an XY sizer container. */
component *xysizer_create(void);

/** @brief Attach a child component at the specified position and size. */
void xysizer_attach(component *sizer, component *c, int x, int y, int w, int h);

/** @brief Set the user data associated with this sizer. */
void xysizer_set_userdata(component *sizer, void *userdata);

/** @brief Get the user data associated with this sizer. */
void *xysizer_get_userdata(component *sizer);

#endif // XYSIZER_H
