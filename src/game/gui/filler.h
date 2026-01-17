/**
 * @file filler.h
 * @brief GUI filler component
 * @details A filler component that takes up space in sizers without rendering anything.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef FILLER_H
#define FILLER_H

#include "game/gui/component.h"

/**
 * @brief Create a filler component
 * @details Creates an invisible component used to fill space in layouts.
 * @return Pointer to the newly created filler component
 */
component *filler_create(void);

#endif // FILLER_H
