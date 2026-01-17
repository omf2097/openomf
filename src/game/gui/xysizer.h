/**
 * @file xysizer.h
 * @brief XY coordinate-based sizer
 * @details A sizer that places components at absolute X/Y coordinates with explicit dimensions.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef XYSIZER_H
#define XYSIZER_H

#include "game/gui/component.h"

/**
 * @brief Create an XY sizer
 * @return Pointer to the newly created XY sizer component
 */
component *xysizer_create(void);

/**
 * @brief Attach a component to the XY sizer at specified position and size
 * @param sizer XY sizer to attach to
 * @param c Component to attach
 * @param x X coordinate for the component
 * @param y Y coordinate for the component
 * @param w Width for the component
 * @param h Height for the component
 */
void xysizer_attach(component *sizer, component *c, int x, int y, int w, int h);

/**
 * @brief Set user data for the sizer
 * @param sizer XY sizer to modify
 * @param userdata User data pointer to store
 */
void xysizer_set_userdata(component *sizer, void *userdata);

/**
 * @brief Get user data from the sizer
 * @param sizer XY sizer to query
 * @return Stored user data pointer
 */
void *xysizer_get_userdata(component *sizer);

#endif // XYSIZER_H
