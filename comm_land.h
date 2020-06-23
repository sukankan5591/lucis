#ifndef _COMM_LAND_H_
#define _COMM_LAND_H_

int   tty_raw(int);
int   tty_reset(int);
void  tty_atexit(void);
void  sig_winsize(int);
void  sig_exit(int);
void  sig_pipe(int);
void  *stdin_to_sock(void *);
void  *sock_to_stdout(void *);
void  usage(const char *);
void  version(const char *);

#endif
