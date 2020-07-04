#define _XOPEN_SOURCE 600
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>

#include "comm.h"
#include "comm_halo.h"


extern int         ptyfd;
extern int         sockfd;
extern pid_t       pid;
extern struct      sockaddr_in  seraddr;
extern struct      sockaddr_in  locaddr;
extern struct      winsize      wsize;  
extern struct      cmd_t        cmd;


int ptym_open(char *pts_name, int pts_namesz)
{
    char *ptr = NULL;
    int fdm;

    fdm = posix_openpt(O_RDWR | O_NOCTTY);
    if(fdm < 0)
    {
        perror("posix_openpt");
        return -1;
    }

    if(grantpt(fdm) < 0)
    {
        perror("grantpt");
        close(fdm);
        return -2;
    }

    if(unlockpt(fdm) < 0)
    {
        perror("unlockpt");
        close(fdm);
        return -3;
    }

    ptr = ptsname(fdm);
    if(ptr == NULL)
    {
        perror("ptsname");
        close(fdm);
        return -4;
    }

    strncpy(pts_name, ptr, pts_namesz);
    pts_name[pts_namesz -1] = '\0';

    return fdm;
}


int ptys_open(char *pts_name)
{
    int fds;

    fds = open(pts_name, O_RDWR);
    if(fds < 0)
        return -1;

    return fds;
}


pid_t pty_fork( int *ptrfdm, char *slave_name, 
                long int slave_namesz,
                const struct termios *slave_termios,
                const struct winsize *slave_winsize)
{
    int fdm, fds;
    pid_t pid;
    char pts_name[20] = "";

    fdm = ptym_open(pts_name, sizeof(pts_name));
    if(fdm < 0)
    {
        printf("can't open pty fd:%d\n", fdm);
        return -1;
    }

    if(slave_name != NULL)
        strncpy(slave_name, pts_name, slave_namesz);

    pid = fork();
    if(pid < 0)
        return -1;
    
    if(pid == 0)
    {
        if(setsid() < 0)
            perror("setsid error"),exit(-1);
    /*
    Solve the problem that the effective user ID of shell child processes and 
    parent processes are different when general users run programs with SUID permissions
    */
        if(geteuid() == 0 && getuid() != 0)
            setuid(0);

        fds = ptys_open(pts_name);
        if(fds < 0)
            perror("can't open slave pty"),exit(-1);
        close(fdm);

        if(slave_termios != NULL)
            if(tcsetattr(fds, TCSANOW, slave_termios) < 0)
                perror("tcsetattr error on slave pty"),exit(-1);

        if(slave_winsize != NULL)
            if(ioctl(fds, TIOCSWINSZ, slave_winsize) < 0)
                perror("ioctl TIOCSWINSZ error on slave pty"),exit(-1);

        if(dup2(fds, STDIN_FILENO) != STDIN_FILENO)
            perror("dup2 stdin"),exit(-1);
        if(dup2(fds, STDOUT_FILENO) != STDOUT_FILENO)
            perror("dup2 stdout"),exit(-1);
        if(dup2(fds, STDERR_FILENO) != STDERR_FILENO)
            perror("dup2 stderr"),exit(-1);
        if((fds != STDIN_FILENO) && (fds != STDOUT_FILENO) && (fds != STDERR_FILENO))
            close(fds);

        return 0;

    }else{
        *ptrfdm = fdm;
        return pid;
    }
}


void *sock_to_pty(void *arg)
{
    int nread, nwrite;
    while(1)
    {
        nread = recv(sockfd, &cmd, sizeof(cmd), MSG_WAITALL);
        if(nread <= 0)
            break;
            
        if(cmd.flag & SET_WINDOW)
        {
            wsize.ws_row = cmd.row;
            wsize.ws_col = cmd.col;
        	ioctl(ptyfd, TIOCSWINSZ, &wsize);
        }

        if(cmd.flag & SET_KEY)
        {
            nwrite = write(ptyfd, cmd.key, cmd.size);
            if(nwrite <= 0)
                break;
        }
        cmd.flag &= 0;
        
    }
    kill(getpid(), SIGTERM);
    pthread_exit(0);
    return NULL;
}


void *pty_to_sock(void *arg)
{
    int    nread, nwrite;
    char   buf[BUFSIZE];
    while(1)
    {
        nread = read(ptyfd, buf, BUFSIZE);
        if(nread <= 0)
            break;
            
        nwrite = write(sockfd, buf, nread);
        if(nwrite < 0)
            break;
    }
    kill(getpid(), SIGTERM);
    pthread_exit(0);
    return NULL;
}


void sig_child(int signo)
{
    waitpid(pid, NULL, WNOHANG);
    close(sockfd);
    close(ptyfd);
    exit(0);
}


void sig_exit(int signo)
{
    close(sockfd);
    close(ptyfd);
    exit(0);
}


void sig_pipe(int signo)
{
    kill(pid, SIGKILL);
    waitpid(pid, NULL, WNOHANG);
    close(sockfd);
    close(ptyfd);
    exit(127);
}


void usage(const char *name)
{
    printf(
    "usage: %s [-l ip] [-t port] -s <ip> -p <port>\n"
    "    -l  bind local IP address\n"
    "    -t  bind local port\n"
    "    -s  remote IP or domain name\n"
    "    -p  remote host port\n"
    "    -h  display this help\n"
    "    -v  show version\n", name);
}


void version(const char *name)
{
    printf("%s  version  %s\n", name, VERSION);
}
