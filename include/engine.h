#ifndef _ENGINE_H
#define _ENGINE_H

int engine_init(); // Init window, audiodevice, etc.
void engine_run(int net_mode); // Run game
void engine_close(); // Kill window, audiodev

#endif // _ENGINE_H
