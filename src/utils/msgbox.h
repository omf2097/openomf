/**
 * @file msgbox.h
 * @brief Error message box display.
 * @details Provides a platform-independent way to display error messages
 *          in a graphical dialog box (using SDL).
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef MSGBOX_H
#define MSGBOX_H

#include "utils/compat.h"

/**
 * @brief Display an error message box.
 * @details Shows a modal dialog with the formatted error message.
 *          Uses SDL_ShowSimpleMessageBox internally.
 * @param fmt Printf-style format string
 * @param ... Format arguments
 */
void err_msgbox(const char *fmt, ...) ATTR_FORMAT_PRINTF(1, 2);

#endif // MSGBOX_H
