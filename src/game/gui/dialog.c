#include "game/gui/dialog.h"
#include "game/gui/button.h"
#include "game/gui/label.h"
#include "game/gui/menu.h"
#include "game/gui/menu_background.h"
#include "video/video.h"
#include <string.h>

void dialog_cancel(component *c, void *userdata) {
    dialog *dlg = userdata;
    if(dlg->clicked) {
        dlg->clicked(dlg, DIALOG_RESULT_CANCEL);
    }
}

void dialog_no(component *c, void *userdata) {
    dialog *dlg = userdata;
    if(dlg->clicked) {
        dlg->clicked(dlg, DIALOG_RESULT_NO);
    }
}

void dialog_yes_ok(component *c, void *userdata) {
    dialog *dlg = userdata;
    if(dlg->clicked) {
        dlg->clicked(dlg, DIALOG_RESULT_YES_OK);
    }
}

void dialog_create_with_tconf(dialog *dlg, dialog_style style, text_settings *tconf, text_settings *tconf_desc,
                              const char *text, int x, int y) {
    dlg->x = x;
    dlg->y = y;
    dlg->visible = 0;
    dlg->userdata = NULL;
    dlg->clicked = NULL;

    int w = NATIVE_W - 2 * x;

    component *menu = menu_create(11);

    menu_attach(menu, label_create_with_width(tconf_desc, text, w));

    component *menu2 = menu_create(11);

    menu_set_horizontal(menu2, true);
    menu_set_background(menu2, false);
    menu_set_margin_top(menu2, 0);
    menu_set_padding(menu2, 20);
    menu_set_centered(menu2, true);
    menu_attach(menu, menu2);

    if(style == DIALOG_STYLE_CANCEL) {
        component *cancel = button_create(tconf, "CANCEL", NULL, COM_ENABLED, dialog_cancel, dlg);
        button_set_border(cancel, TEXT_MEDIUM_GREEN);
        menu_attach(menu2, cancel);
    } else if(style == DIALOG_STYLE_YES_NO) {
        component *yes = button_create(tconf, "YES", NULL, COM_ENABLED, dialog_yes_ok, dlg);
        component *no = button_create(tconf, "NO", NULL, COM_ENABLED, dialog_no, dlg);
        button_set_border(yes, TEXT_MEDIUM_GREEN);
        button_set_border(no, TEXT_MEDIUM_GREEN);
        menu_attach(menu2, yes);
        menu_attach(menu2, no);
    } else if(style == DIALOG_STYLE_OK) {
        component *ok = button_create(tconf, "OK", NULL, COM_ENABLED, dialog_yes_ok, dlg);
        button_set_border(ok, TEXT_MEDIUM_GREEN);
        menu_attach(menu2, ok);
    }

    dlg->frame = gui_frame_create(x, y, w, 80);
    gui_frame_set_root(dlg->frame, menu);
    gui_frame_layout(dlg->frame);
}

void dialog_create(dialog *dlg, dialog_style style, const char *text, int x, int y) {

    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_BIG;
    tconf.cforeground = TEXT_MEDIUM_GREEN;

    text_settings tconf_desc;
    text_defaults(&tconf_desc);
    tconf_desc.font = FONT_BIG;
    tconf_desc.cforeground = TEXT_BRIGHT_GREEN;
    tconf_desc.halign = TEXT_CENTER;

    dialog_create_with_tconf(dlg, style, &tconf, &tconf_desc, text, x, y);
}

void dialog_free(dialog *dlg) {
    gui_frame_free(dlg->frame);
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
    gui_frame_render(dlg->frame);
}

void dialog_tick(dialog *dlg) {
    if(!dlg->visible) {
        return;
    }
    gui_frame_tick(dlg->frame);
}

void dialog_event(dialog *dlg, int action) {
    if(!dlg->visible) {
        return;
    }

    gui_frame_action(dlg->frame, action);

    if(action == ACT_ESC && dlg->clicked) {
        dlg->clicked(dlg, DIALOG_RESULT_CANCEL);
    }
}
