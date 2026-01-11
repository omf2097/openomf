/**
 * @file msgbox.h
 * @brief Error message box display.
 * @details Provides a platform-independent way to display error messages
 *          in a graphical dialog box (using SDL).
 */

#ifndef MSGBOX_H
#define MSGBOX_H

/**
 * @brief Display an error message box.
 * @details Shows a modal dialog with the formatted error message.
 *          Uses SDL_ShowSimpleMessageBox internally.
 * @param fmt Printf-style format string
 * @param ... Format arguments
 */
void err_msgbox(const char *fmt, ...);

#endif // MSGBOX_H
