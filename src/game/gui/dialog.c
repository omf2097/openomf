#include "game/gui/dialog.h"
#include "controller/controller.h"
#include "game/gui/button.h"
#include "game/gui/filler.h"
#include "game/gui/label.h"
#include "game/gui/menu.h"
#include "video/video.h"

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

void dialog_create_h(dialog *dlg, dialog_style style, const char *text, int x, int y, int h) {
    gui_theme theme;
    gui_theme_defaults(&theme);
    theme.dialog.border_color = TEXT_MEDIUM_GREEN;
    theme.text.font = FONT_BIG;
    theme.text.primary_color = TEXT_MEDIUM_GREEN;
    theme.text.secondary_color = TEXT_BRIGHT_GREEN;
    theme.text.active_color = TEXT_BRIGHT_GREEN;
    theme.text.inactive_color = TEXT_MEDIUM_GREEN;

    dlg->x = x;
    dlg->y = y;
    dlg->visible = 0;
    dlg->userdata = NULL;
    dlg->clicked = NULL;

    int w = NATIVE_W - 2 * x;

    component *menu = menu_create();

    component *title = label_create_title(text);
    label_set_margin(title, (text_margin){2, 2, 2, 2});
    menu_attach(menu, title);
    menu_attach(menu, filler_create());

    component *menu2 = menu_create();

    menu_set_horizontal(menu2, true);
    menu_set_background(menu2, false);
    menu_set_centered(menu2, true);
    menu_set_margin_top(menu2, 0);
    menu_set_padding(menu2, 20);
    menu_attach(menu, menu2);
    component_set_size_hints(menu2, -1, 12);

    if(style == DIALOG_STYLE_CANCEL) {
        menu_attach(menu2, button_create("CANCEL", NULL, false, true, dialog_cancel, dlg));
    } else if(style == DIALOG_STYLE_YES_NO) {
        menu_attach(menu2, button_create("YES", NULL, false, true, dialog_yes_ok, dlg));
        menu_attach(menu2, button_create("NO", NULL, false, true, dialog_no, dlg));
    } else if(style == DIALOG_STYLE_OK) {
        menu_attach(menu2, button_create("OK", NULL, false, true, dialog_yes_ok, dlg));
    }

    dlg->frame = gui_frame_create(&theme, x, y, w, h);
    gui_frame_set_root(dlg->frame, menu);
    gui_frame_layout(dlg->frame);
}

void dialog_create(dialog *dlg, dialog_style style, const char *text, int x, int y) {
    dialog_create_h(dlg, style, text, x, y, 60);
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
