#ifndef DIALOG_H
#define DIALOG_H

#include "game/gui/component.h"
#include "utils/vector.h"
#include "video/surface.h"

typedef enum dialog_style_t
{
    DIALOG_STYLE_YES_NO,
    DIALOG_STYLE_OK
} dialog_style;

typedef enum dialog_result_t
{
    DIALOG_RESULT_CANCEL,
    DIALOG_RESULT_YES_OK,
    DIALOG_RESULT_NO
} dialog_result;

typedef struct component_t component;
typedef struct dialog_t dialog;

typedef void (*dialog_clicked_cb)(dialog *, dialog_result result);

typedef struct dialog_t {
    int x;
    int y;
    char text[256];
    surface background;
    component *yes;
    component *no;
    component *ok;
    int visible;
    dialog_result result;

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
