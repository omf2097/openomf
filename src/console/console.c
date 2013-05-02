#include "console/console.h"
#include "video/video.h"
#include "game/menu/menu_background.h"
#include "utils/log.h"
#include <stdlib.h>

console *con = NULL;

int console_init() {
    if(con != NULL) return 1;
    con = malloc(sizeof(console));
    con->isopen = 0;
    con->ypos = 0;
    con->ticks = 0;
    con->input[0] = '\0';
    menu_background_create(&con->background, 322, 101);
 
    // Create font
    font_create(&con->font);
    if(font_load(&con->font, "resources/CHARSMAL.DAT", FONT_SMALL)) {
        PERROR("Error while loading small font!");
        font_free(&con->font);
        return 1;
    }
    return 0;
}

void console_close() {
    texture_free(&con->background);
    font_free(&con->font);
    free(con);
}

void console_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN) {
        unsigned char code = e->key.keysym.sym;
        unsigned char len = strlen(con->input);
        /*if ((code >= SDLK_a && code <= SDLK_z) || (code >= SDLK_0 && code <= SDLK_9) || code == SDLK_SPACE || code == SDLK) {*/
        if (code >= 32 && code <= 126) {
            if (len < 40) {
                con->input[len+1] = '\0';
                con->input[len] = code;
            }
        } else if (code == SDLK_BACKSPACE || code == SDLK_DELETE) {
            if (len > 0) {
                con->input[len-1] = '\0';
            }
        } else if (code == SDLK_RETURN) {
            // send the input somewhere and clear the input line
            con->input[0] = '\0';
        }
    }
}

void console_render() {
    if (con->ypos > 0) {
        video_render_sprite(&con->background, -1, con->ypos - 101, BLEND_ALPHA_FULL);
        int t = con->ticks / 2;
        // input line
        font_render(&con->font, con->input, 0 , con->ypos - 7, 121, 121, 121);
        //cursor
        font_render(&con->font, "", strlen(con->input)*8 , con->ypos - 7, 121 - t, 121 - t, 121 - t);
    }
}

void console_tick() {
    if (con->isopen && con->ypos < 100) {
        con->ypos+=4;
    } else if (!con->isopen && con->ypos > 0) {
        con->ypos-=4;
    }
    if(!con->dir) {
        con->ticks++;
    } else {
        con->ticks--;
    }
    if(con->ticks > 120) {
        con->dir = 1;
    }
    if(con->ticks == 0) {
        con->dir = 0;
    }
}

int console_window_is_open() {
    return con->isopen;
}

void console_window_open() {
    con->isopen = 1;
}

void console_window_close() {
    con->isopen = 0;
}
