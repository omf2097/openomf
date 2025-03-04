#ifndef DIALOG_H
#define DIALOG_H

#include "game/gui/component.h"
#include "game/gui/gui_frame.h"
#include "utils/vector.h"
#include "video/surface.h"

typedef enum dialog_style
{
    DIALOG_STYLE_YES_NO,
    DIALOG_STYLE_OK,
    DIALOG_STYLE_CANCEL
} dialog_style;

typedef enum dialog_result
{
    DIALOG_RESULT_CANCEL,
    DIALOG_RESULT_YES_OK,
    DIALOG_RESULT_NO
} dialog_result;

typedef struct component component;
typedef struct dialog dialog;

typedef void (*dialog_clicked_cb)(dialog *, dialog_result result);

typedef struct dialog {
    int x;
    int y;
    gui_frame *frame;
    int visible;

    // events
    void *userdata;
    dialog_clicked_cb clicked;
} dialog;

void dialog_create(dialog *dlg, dialog_style style, const char *text, int x, int y);
void dialog_free(dialog *dlg);
void dialog_show(dialog *dlg, int visible);
int dialog_is_visible(dialog *dlg);

void dialog_tick(dialog *dlg);
void dialog_render(dialog *dlg);
void dialog_event(dialog *dlg, int action);

#endif // DIALOG_H
