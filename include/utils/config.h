#ifndef _CONFIG_H
#define _CONFIG_H

int conf_init(const char *filename);
int conf_write_config(const char *filename);
void conf_close();

int conf_int(const char *name);
double conf_float(const char *name);
int conf_bool(const char *name);
const char* conf_string(const char *name);

#endif