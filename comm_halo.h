#ifndef _COMM_HALO_H_
#define _COMM_HALO_H_

int    ptym_open(char *, int);
int    ptys_open(char *);
pid_t  pty_fork(int *, char *, long int, const struct termios *, const struct winsize *);
void  *pty_to_sock(void *);
void  *sock_to_pty(void *);
void  sig_exit(int);
void  sig_child(int);
void  sig_pipe(int);
void  usage(const char *);
void  version(const char *);

#endif
