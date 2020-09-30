#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <libgen.h>
#include <pthread.h>
#include <netinet/tcp.h>  // head file for: TCP_NODELAY

#include "comm.h"
#include "comm_land.h"

#define     OPTIONS      "l:p:hv" 

struct      sockaddr_in seraddr;
struct      sockaddr_in cliaddr;
int         listenfd;
int         connfd;
char const  *cli_IP; 
int         cli_PORT;
char        str[INET_ADDRSTRLEN]="\0";
pthread_t   intosok, soktoin;
cmd_t       cmd;
struct      winsize wsize;


int main(int argc, char *argv[])
{
	int ch, ret;
    int on = 1;
    socklen_t addr_len;
    char loc_ip[20]  = "";    unsigned char islocip_seted = 0;
    char loc_port[8] = "";    unsigned char islocport_seted = 0;
    

    if(!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
        printf("stdin or stdout not tty\n"),exit(-1);

	while((ch = getopt(argc, argv, OPTIONS))!= -1)
	{
		switch(ch)
		{
			case 'l': 
                strncpy(loc_ip, optarg, sizeof(loc_ip));
                islocip_seted =1;
                break;
			case 'p': 
                strncpy(loc_port, optarg, sizeof(loc_port));
                islocport_seted = 1;
                break;
			case 'h': 
                usage(basename(argv[0]));
                exit(0);
                break;
			case 'v': 
                version(basename(argv[0]));
                exit(0);
                break;
			default: 
                printf("please use -h for more help.\n");
                exit(1);
		}
	}

    signal(SIGHUP,   SIG_IGN);
    signal(SIGINT,   sig_exit);
    signal(SIGQUIT,  sig_exit);
    signal(SIGTERM,  sig_exit);
    signal(SIGPIPE,  sig_pipe);
    signal(SIGTSTP,  SIG_IGN);
    signal(SIGTTIN,  SIG_IGN);
    signal(SIGTTOU,  SIG_IGN);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    addr_len = sizeof(seraddr);
    setsockopt(listenfd, SOL_SOCKET,  SO_REUSEADDR, &on, sizeof(on));
    setsockopt(listenfd, SOL_SOCKET,  SO_KEEPALIVE, &on, sizeof(on));
    setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY,  &on, sizeof(on));
    
    seraddr.sin_family = AF_INET;
    if(islocip_seted)
        inet_pton(AF_INET, loc_ip, &seraddr.sin_addr);
    else
        seraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(!islocport_seted)
    {
        usage(basename(argv[0]));
        exit(1);
    }
    seraddr.sin_port = htons((unsigned short)atoi(loc_port));

    ret = bind(listenfd, (struct sockaddr *)&seraddr, sizeof(seraddr));
    if(ret != 0)
    {
        perror("bind");
        usage(basename(argv[0]));
        exit(1);
    }

    listen(listenfd, 1);
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &addr_len);
    
    close(listenfd);
    signal(SIGWINCH, sig_winsize);
    sig_winsize(SIGWINCH);

    cli_IP = inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str));
    cli_PORT = ntohs(cliaddr.sin_port);
    printf("Connected From %s:%d\n", cli_IP, cli_PORT);

    if(tty_raw(STDIN_FILENO) < 0)
        printf("tty raw error"),exit(1);

    atexit(tty_atexit);
    pthread_create(&intosok, NULL, stdin_to_sock, NULL);
    pthread_create(&soktoin, NULL, sock_to_stdout, NULL);

    while(1)
        pause();

    return 0;
}
