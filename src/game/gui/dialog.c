#include "game/gui/dialog.h"
#include "game/gui/menu_background.h"
#include "game/gui/textbutton.h"
#include "video/video.h"
#include <string.h>

#define MAX_WIDTH 160

void dialog_create(dialog *dlg, dialog_style style, const char *text, int x, int y) {
    int w, h;

    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_BIG;
    tconf.cforeground = COLOR_DARK_GREEN;

    text_settings tconf_desc;
    text_defaults(&tconf_desc);
    tconf_desc.font = FONT_SMALL;
    tconf_desc.cforeground = COLOR_DARK_GREEN;

    dlg->x = x;
    dlg->y = y;
    dlg->yes = NULL;
    dlg->no = NULL;
    dlg->ok = NULL;
    dlg->visible = 0;
    dlg->userdata = NULL;
    dlg->clicked = NULL;
    strncpy(dlg->text, text, sizeof(dlg->text) - 1);
    dlg->text[sizeof(dlg->text) - 1] = 0;
    font_get_wrapped_size_shadowed(&font_small, dlg->text, MAX_WIDTH, TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM, &w, &h);
    int tsize = text_char_width(&tconf);
    menu_background_create(&dlg->background, MAX_WIDTH + 30, h + 24 + tsize);

    if(style == DIALOG_STYLE_YES_NO) {
        dlg->yes = textbutton_create(&tconf, "YES", NULL, COM_ENABLED, NULL, NULL);
        dlg->no = textbutton_create(&tconf, "NO", NULL, COM_ENABLED, NULL, NULL);
        textbutton_set_border(dlg->yes, COLOR_BLUE);
        textbutton_set_border(dlg->no, COLOR_BLUE);
        component_layout(dlg->yes, x + 54, x + h + 6, 8, 8);
        component_layout(dlg->no, x + 114, x + h + 6, 8, 8);
        component_select(dlg->yes, 1);
        dlg->result = DIALOG_RESULT_YES_OK;
    } else if(style == DIALOG_STYLE_OK) {
        dlg->ok = textbutton_create(&tconf, "OK", NULL, COM_ENABLED, NULL, NULL);
        textbutton_set_border(dlg->ok, COLOR_BLUE);
        component_layout(dlg->ok, x + 84, x + h + 6, 8, 8);
        component_select(dlg->ok, 1);
        dlg->result = DIALOG_RESULT_YES_OK;
    }
}

void dialog_free(dialog *dlg) {
    if(dlg->yes) {
        component_free(dlg->yes);
    }
    if(dlg->no) {
        component_free(dlg->no);
    }
    if(dlg->ok) {
        component_free(dlg->ok);
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
    if(!dlg->visible) {
        return;
    }
    video_draw(&dlg->background, dlg->x, dlg->y);
    if(dlg->yes) {
        component_render(dlg->yes);
    }
    if(dlg->no) {
        component_render(dlg->no);
    }
    if(dlg->ok) {
        component_render(dlg->ok);
    }
    font_render_wrapped_shadowed(&font_small, dlg->text, dlg->x + 15, dlg->y + 3, MAX_WIDTH, COLOR_GREEN,
                                 TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM);
}

void dialog_tick(dialog *dlg) {
    if(!dlg->visible) {
        return;
    }
    if(dlg->yes) {
        component_tick(dlg->yes);
    }
    if(dlg->no) {
        component_tick(dlg->no);
    }
    if(dlg->ok) {
        component_tick(dlg->ok);
    }
}

void dialog_event(dialog *dlg, int action) {
    if(!dlg->visible) {
        return;
    }
    if(action == ACT_LEFT || action == ACT_RIGHT) {
        if(dlg->yes && dlg->no) {
            if(component_is_selected(dlg->yes)) {
                component_select(dlg->yes, 0);
                component_select(dlg->no, 1);
                dlg->result = DIALOG_RESULT_NO;
            } else if(component_is_selected(dlg->no)) {
                component_select(dlg->yes, 1);
                component_select(dlg->no, 0);
                dlg->result = DIALOG_RESULT_YES_OK;
            }
        }
    } else if(action == ACT_PUNCH || action == ACT_KICK) {
        if(dlg->clicked) {
            dlg->clicked(dlg, dlg->result);
        }
        dlg->visible = 0;
    } else if(action == ACT_ESC) {
        if(dlg->clicked) {
            dlg->clicked(dlg, DIALOG_RESULT_CANCEL);
        }
        dlg->visible = 0;
    }
}
