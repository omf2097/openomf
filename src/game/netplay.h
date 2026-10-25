#ifndef NETPLAY_H
#define NETPLAY_H

#include <stdbool.h>

typedef struct scene_t scene;
typedef struct component component;
typedef struct controller_t controller;

bool netplay_enabled(void);
bool netplay_init(void);
void netplay_quit(void);

int netplay_lobby_create(scene *scene);
component *netplay_menu_connect_create(scene *s);
component *netplay_menu_listen_create(scene *s);
void netplay_set_winner(controller *ctrl, int winner);
void netplay_menu_hook(controller *ctrl, int action);

#endif // NETPLAY_H
