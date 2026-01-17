/**
 * @file dialog.h
 * @brief GUI dialog box
 * @details Modal dialog boxes with configurable buttons (Yes/No, OK, Cancel).
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef DIALOG_H
#define DIALOG_H

#include "game/gui/component.h"
#include "game/gui/gui_frame.h"
#include "utils/vector.h"
#include "video/surface.h"

/**
 * @brief Dialog button styles
 */
typedef enum dialog_style
{
    DIALOG_STYLE_YES_NO, ///< Yes and No buttons
    DIALOG_STYLE_OK,     ///< OK button only
    DIALOG_STYLE_CANCEL  ///< Cancel button only
} dialog_style;

/**
 * @brief Dialog result values
 */
typedef enum dialog_result
{
    DIALOG_RESULT_CANCEL, ///< Cancel or dialog dismissed
    DIALOG_RESULT_YES_OK, ///< Yes or OK pressed
    DIALOG_RESULT_NO      ///< No pressed
} dialog_result;

typedef struct component component;
typedef struct dialog dialog;

typedef void (*dialog_clicked_cb)(dialog *, dialog_result result); ///< Dialog button click callback

/**
 * @brief Dialog structure
 */
typedef struct dialog {
    int x;            ///< X coordinate of the dialog
    int y;            ///< Y coordinate of the dialog
    gui_frame *frame; ///< GUI frame containing dialog contents
    int visible;      ///< Whether the dialog is visible

    void *userdata;            ///< User data for callbacks
    dialog_clicked_cb clicked; ///< Callback when a button is clicked
} dialog;

/**
 * @brief Create a dialog with custom height
 * @param dlg Dialog structure to initialize
 * @param style Button style for the dialog
 * @param text Message text to display
 * @param x X coordinate for the dialog
 * @param y Y coordinate for the dialog
 * @param h Height of the dialog
 */
void dialog_create_h(dialog *dlg, dialog_style style, const char *text, int x, int y, int h);

/**
 * @brief Create a dialog with automatic height
 * @param dlg Dialog structure to initialize
 * @param style Button style for the dialog
 * @param text Message text to display
 * @param x X coordinate for the dialog
 * @param y Y coordinate for the dialog
 */
void dialog_create(dialog *dlg, dialog_style style, const char *text, int x, int y);

/**
 * @brief Free the dialog and its resources
 * @param dlg Dialog to free
 */
void dialog_free(dialog *dlg);

/**
 * @brief Show or hide the dialog
 * @param dlg Dialog to modify
 * @param visible Non-zero to show, zero to hide
 */
void dialog_show(dialog *dlg, int visible);

/**
 * @brief Check if the dialog is visible
 * @param dlg Dialog to query
 * @return Non-zero if visible
 */
int dialog_is_visible(dialog *dlg);

/**
 * @brief Process a tick update for the dialog
 * @param dlg Dialog to tick
 */
void dialog_tick(dialog *dlg);

/**
 * @brief Render the dialog
 * @param dlg Dialog to render
 */
void dialog_render(dialog *dlg);

/**
 * @brief Handle an action event
 * @param dlg Dialog to receive the action
 * @param action Action code to process
 */
void dialog_event(dialog *dlg, int action);

#endif // DIALOG_H
