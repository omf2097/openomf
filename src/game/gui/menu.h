#ifndef MENU_H
#define MENU_H

#include "game/gui/component.h"
#include "game/gui/gui_frame.h"
#include "video/surface.h"

typedef void (*menu_tick_cb)(component *c);
typedef void (*menu_free_cb)(component *c);
typedef void (*menu_submenu_done_cb)(component *menu, component *submenu);

typedef struct menu {
    surface *bg1;
    surface *bg2;
    surface *help_bg1;
    surface *help_bg2;
    int selected;
    int margin_top;
    int padding;
    bool finished;
    bool horizontal;
    bool background;
    bool centered;
    bool is_submenu;

    int help_x;
    int help_y;
    int help_w;
    int help_h;
    vga_index help_text_color;
    text_horizontal_align help_text_halign;
    text_vertical_align help_text_valign;
    font_size help_text_font;

    char prev_submenu_state;
    component *submenu;
    menu_submenu_done_cb submenu_done;

    void *userdata;
    menu_free_cb free;
    menu_tick_cb tick;
} menu;

component *menu_create(void);
void menu_attach(component *menu, component *c);
void menu_select(component *menu, component *c);
component *menu_selected(const component *menu);
int menu_is_finished(const component *menu);

void menu_set_submenu(component *menu, component *submenu);
void menu_link_menu(component *menu, gui_frame *linked_menu);
component *menu_get_submenu(const component *menu);
void menu_set_submenu_done_cb(component *menu, menu_submenu_done_cb done_cb);

void menu_set_userdata(component *menu, void *userdata);
void *menu_get_userdata(const component *menu);
void menu_set_free_cb(component *menu, menu_free_cb cb);
void menu_set_tick_cb(component *menu, menu_tick_cb cb);
void menu_set_horizontal(component *c, bool horizontal);
void menu_set_background(component *c, bool background);
void menu_set_help_pos(component *c, int x, int y, int h, int w);
void menu_set_help_text_settings(component *c, font_size font, text_horizontal_align halign, vga_index help_text_color);
void menu_set_margin_top(component *c, int margin);
void menu_set_padding(component *c, int padding);
void menu_set_centered(component *c, bool centered);

#endif // MENU_H
