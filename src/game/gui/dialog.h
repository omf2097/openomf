/*! @file
 * @brief Modal dialog with pre-configured button layouts.
 */

#ifndef DIALOG_H
#define DIALOG_H

#include "game/gui/component.h"
#include "game/gui/gui_frame.h"

/** @brief Dialog button styles. */
typedef enum dialog_style
{
    DIALOG_STYLE_YES_NO, ///< Yes and No buttons.
    DIALOG_STYLE_OK,     ///< OK button only.
    DIALOG_STYLE_CANCEL  ///< Cancel button only.
} dialog_style;

/** @brief Dialog result codes. */
typedef enum dialog_result
{
    DIALOG_RESULT_CANCEL, ///< Cancel/No pressed, or ESC.
    DIALOG_RESULT_YES_OK, ///< Yes or OK pressed.
    DIALOG_RESULT_NO      ///< No pressed.
} dialog_result;

typedef struct component component;
typedef struct dialog dialog;

/** @brief Callback invoked when a dialog button is clicked. */
typedef void (*dialog_clicked_cb)(dialog *, dialog_result result);

/** @brief Dialog data structure. */
typedef struct dialog {
    int x;            ///< X position.
    int y;            ///< Y position.
    gui_frame *frame; ///< Contains the menu tree.
    int visible;      ///< Visibility flag.

    // events
    void *userdata;            ///< User data.
    dialog_clicked_cb clicked; ///< Click callback.
} dialog;

/** @brief Create a dialog with custom height. */
void dialog_create_h(dialog *dlg, dialog_style style, const char *text, int x, int y, int h);

/** @brief Create a dialog with default height. */
void dialog_create(dialog *dlg, dialog_style style, const char *text, int x, int y);

/** @brief Free dialog resources. */
void dialog_free(dialog *dlg);

/** @brief Show or hide the dialog. */
void dialog_show(dialog *dlg, int visible);

/** @brief Check if dialog is visible. */
int dialog_is_visible(dialog *dlg);

/** @brief Update dialog animations. */
void dialog_tick(dialog *dlg);

/** @brief Render the dialog. */
void dialog_render(dialog *dlg);

/** @brief Handle action events. */
void dialog_event(dialog *dlg, int action);

#endif // DIALOG_H
