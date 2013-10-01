#ifndef _LOG_H
#define _LOG_H

#ifdef DEBUGMODE
#define DEBUG(...) log_print('D', __VA_ARGS__ )
#else
#define DEBUG(...) 
#endif
#define PERROR(...) log_print('E', __VA_ARGS__ )

void log_print(char mode, char *fmt, ...);
int log_init(const char *filename);
void log_close();

#endif // _LOG_H
