#ifndef _CONFIG_H
#define _CONFIG_H

void conf_addint(char *name, int default_val);
void conf_addbool(char *name, int default_val);
void conf_addfloat(char *name, double default_val);
void conf_addstring(char *name, char *default_val);

int conf_init(const char *filename);
int conf_write_config(const char *filename);
void conf_close();

int conf_int(const char *name);
double conf_float(const char *name);
int conf_bool(const char *name);
const char* conf_string(const char *name);

void conf_setint(const char *name, int val);
void conf_setfloat(const char *name, double val);
void conf_setbool(const char *name, int val);
void conf_setstring(const char *name, const char *val);

#endif
