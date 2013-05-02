#include "console/console.h"
#include <stdlib.h>

console *con = NULL;

int console_init() {
    if(con != NULL) return 1;
    // Remember to create font, too
    return 0;
}

void console_close() {

}

void console_event(SDL_Event *event) {
    // If event gets here, handle it. 
}

void console_render() {
    // Renders console window
}

void console_tick() {
    // Does something. Necessary ?
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
