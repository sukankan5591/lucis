#define _XOPEN_SOURCE 600
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>

#include "comm.h"
#include "comm_land.h"

struct  termios  save_termios;
int     ttysavefd = -1;
enum    {RESET, RAW}  ttystate = RESET;


extern struct      sockaddr_in seraddr;
extern struct      sockaddr_in cliaddr;
extern int         listenfd;
extern int         connfd;
extern pthread_t   intosok, soktoin;
extern cmd_t       cmd;
extern struct      winsize wsize;


int tty_raw(int fd)
{
    int err;
    struct termios buf;
    if(ttystate != RESET)
    {
        errno = EINVAL;
        return -1;
    }
    if(tcgetattr(fd, &buf))
        return -1;

    save_termios = buf;
    buf.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG);
    buf.c_iflag &= ~(BRKINT|ICRNL|INPCK|ISTRIP|IXON);
    buf.c_cflag &= ~(CSIZE|PARENB);
    buf.c_cflag |= CS8;
    buf.c_oflag &= ~(OPOST);
    buf.c_cc[VMIN] = 1;
    buf.c_cc[VTIME] = 0;
    if(tcsetattr(fd, TCSAFLUSH, &buf) < 0)
        return -1;

    if(tcgetattr(fd, &buf) < 0 )
    {
        err = errno;
        tcsetattr(fd, TCSAFLUSH, &save_termios);
        errno = err;
        return -1;
    }

    if((buf.c_lflag & (ECHO|ICANON|IEXTEN|ISIG)) ||
       (buf.c_iflag & (BRKINT|ICRNL|INPCK|ISTRIP|IXON)) ||
       (buf.c_cflag & (CSIZE|PARENB|CS8)) != CS8 ||
       (buf.c_oflag &OPOST) || buf.c_cc[VMIN] != 1 ||
       buf.c_cc[VTIME] != 0)
    {
         tcsetattr(fd, TCSAFLUSH, &save_termios);
         errno = EINVAL;
         return -1;
    }

    ttystate = RAW;
    ttysavefd = fd;
    
    return 0;
}


int tty_reset(int fd)
{
    if(ttystate == RESET)
        return 0;
    
    if(tcsetattr(fd, TCSAFLUSH, &save_termios) < 0)
        return -1;
    
    ttystate = RESET;
    
    return 0;
}


void tty_atexit(void)
{
    if(ttysavefd >= 0)
        tty_reset(ttysavefd);
}


void sig_winsize(int signo)
{
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
    cmd.row   = wsize.ws_row;
    cmd.col   = wsize.ws_col;
    cmd.flag |= SET_WINDOW;
    write(connfd, &cmd, sizeof(cmd));
    cmd.flag &= 0;
}


void sig_exit(int signo)
{
    tty_reset(STDIN_FILENO);
    close(connfd);
    printf("\r");
    exit(0);
}


void sig_pipe(int signo)
{
    tty_reset(STDIN_FILENO);
    close(connfd);
    printf("\r");
    exit(127);
}

void *stdin_to_sock(void *args)
{
    int nread;
    while(1)
    {
        nread = read(STDIN_FILENO, cmd.key, sizeof(cmd.key));
        if(nread == 0)
            break;
        cmd.flag |= SET_KEY;
        cmd.size = nread;
        write(connfd, &cmd, sizeof(cmd));
        cmd.flag &= 0;
    }
    kill(getpid(), SIGTERM);
    pthread_exit(0);
    return NULL;
}


void *sock_to_stdout(void *args)
{
    char buf[1024];
    int nread;
    while(1)
    {
        nread = read(connfd, &buf, 1024);
        if(nread == 0)
            break;
        write(STDOUT_FILENO, buf, nread);
    }
    kill(getpid(), SIGTERM);
    pthread_exit(0);
    return NULL;
}


void usage(const char *name)
{
    printf(
    "usage: %s [-l ip] -p <port>\n"
    "    -l  specify bound native host IP\n"
    "    -p  specify listen port\n"
    "    -h  display this help\n"
    "    -v  show version\n", name);
}


void version(const char *name)
{
    printf("%s  version  %s\n", name, VERSION);
}
