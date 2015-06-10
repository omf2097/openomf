#ifndef _PILOT_HTML_H
#define _PILOT_HTML_H

#include <shadowdive/shadowdive.h>
#include <stdio.h>

void print_pilot_info_html(FILE *f, sd_pilot *pilot);
void print_pilot_player_info_html(FILE *f, sd_pilot *pilot);

#endif // _PILOT_HTML_H
