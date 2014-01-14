#ifndef _LOG_H
#define _LOG_H

#ifdef DEBUGMODE
#define DEBUG(...) log_print('D', __VA_ARGS__ )
#else
#define DEBUG(...) 
#endif

#ifdef STANDALONE_SERVER
#define PERROR(...) log_print('E', __VA_ARGS__ )
#else
#define PERROR(...) log_msgbox('E', __VA_ARGS__ )
void log_msgbox(char mode, const char *fmt, ...);
#endif
#define INFO(...) log_print('I', __VA_ARGS__ )

void log_print(char mode, const char *fmt, ...);
int log_init(const char *filename);
void log_close();

#endif // _LOG_H
