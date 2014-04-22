#include "video/video.h"
#include "utils/log.h"
#include "game/menu/menu_background.h"
#include "game/menu/textbutton.h"
#include "game/menu/dialog.h"

#define MAX_WIDTH 148

void dialog_create(dialog *dlg, dialog_style style, const char *text, int x, int y) {
    int w, h;
    dlg->x = x;
    dlg->y = y;
    dlg->yes = NULL;
    dlg->no = NULL;
    dlg->ok = NULL;
    dlg->visible = 0;
    dlg->userdata = NULL;
    dlg->clicked = NULL;
    strcpy(dlg->text, text);
    font_get_wrapped_size(&font_small, text, MAX_WIDTH, &w, &h);
    menu_background_create(&dlg->background, MAX_WIDTH+30, h+24+font_large.h);

    if(style == DIALOG_STYLE_YES_NO) {
        dlg->yes = malloc(sizeof(component));
        dlg->no = malloc(sizeof(component));
        textbutton_create(dlg->yes, &font_large, "YES");
        textbutton_create(dlg->no, &font_large, "NO");
        textbutton_set_border(dlg->yes, COLOR_BLUE);
        textbutton_set_border(dlg->no, COLOR_BLUE);
        component_layout(dlg->yes, x + 54, x+h+6, 8, 8);
        component_layout(dlg->no, x + 114, x+h+6, 8, 8);
        dlg->yes->selected = 1;
        dlg->result = DIALOG_RESULT_YES;
    } else if(style == DIALOG_STYLE_OK) {
        dlg->ok = malloc(sizeof(component));
        textbutton_create(dlg->ok, &font_large, "OK");
        textbutton_set_border(dlg->ok, COLOR_BLUE);
        component_layout(dlg->ok, x + 84, y + 30, 8, 8);
        dlg->ok->selected = 1;
        dlg->result = DIALOG_RESULT_OK;
    }
}

void dialog_free(dialog *dlg) {
    if(dlg->yes) {
        textbutton_free(dlg->yes);
        free(dlg->yes);
    }
    if(dlg->no) {
        textbutton_free(dlg->no);
        free(dlg->no);
    }
    if(dlg->ok) {
        textbutton_free(dlg->ok);
        free(dlg->ok);
    }

    surface_free(&dlg->background);
}

void dialog_show(dialog *dlg, int visible) {
    dlg->visible = visible;
}

int dialog_is_visible(dialog *dlg) {
    return dlg->visible;
}

void dialog_render(dialog *dlg) {
    if(!dlg->visible) { return; }
    video_render_sprite(&dlg->background, dlg->x, dlg->y, BLEND_ALPHA, 0);
    if(dlg->yes) {
        textbutton_render(dlg->yes);
    }
    if(dlg->no) {
        textbutton_render(dlg->no);
    }
    if(dlg->ok) {
        textbutton_render(dlg->ok);
    }
    font_render_wrapped(&font_small, dlg->text, dlg->x+15, dlg->y+3, MAX_WIDTH, COLOR_GREEN);
}


void dialog_tick(dialog *dlg) {
    if(!dlg->visible) { return; }
    if(dlg->yes) {
        textbutton_tick(dlg->yes);
    }
    if(dlg->no) {
        textbutton_tick(dlg->no);
    }
    if(dlg->ok) {
        textbutton_tick(dlg->ok);
    }
}

void dialog_event(dialog *dlg, int action) {
    if(!dlg->visible) { return; }
    if(action == ACT_LEFT || action == ACT_RIGHT) {
        if(dlg->yes->selected) {
            dlg->yes->selected = 0;
            dlg->no->selected = 1;
            dlg->result = DIALOG_RESULT_NO;
        } else if(dlg->no->selected) {
            dlg->yes->selected = 1;
            dlg->no->selected = 0;
            dlg->result = DIALOG_RESULT_YES;
        }
    } else if(action == ACT_PUNCH || action == ACT_KICK) {
        if(dlg->clicked) {
            dlg->clicked(dlg, dlg->result, dlg->userdata);
        }
        dlg->visible = 0;
    } else if(action == ACT_ESC) {
        dlg->visible = 0;
    }
}
