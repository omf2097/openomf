#ifndef _CONFIG_H
#define _CONFIG_H

int conf_init(const char *filename);
void conf_close();

int conf_int(const char *path);
double conf_float(const char *path);
int conf_bool(const char *path);
const char* conf_string(const char *path);

#endif