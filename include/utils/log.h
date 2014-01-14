#ifndef _LOG_H
#define _LOG_H

#ifdef DEBUGMODE
#define DEBUG(...) log_print('D', __VA_ARGS__ )
#else
#define DEBUG(...) 
#endif
#define PERROR(...) log_msgbox('E', __VA_ARGS__ )
#define INFO(...) log_print('I', __VA_ARGS__ )

#ifdef STANDALONE_SERVER
#define log_msgbox(...) PERROR(__VA_ARGS__)
#else
void log_msgbox(char mode, const char *fmt, ...);
#endif

void log_print(char mode, const char *fmt, ...);
int log_init(const char *filename);
void log_close();

#endif // _LOG_H
