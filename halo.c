#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/unistd.h>
#include <pwd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <pthread.h>
#include <netinet/tcp.h> // head file for: TCP_NODELAY

#include "comm.h"
#include "comm_halo.h"

#define     OPTIONS      "l:t:s:p:hv" 

int         ptyfd;
int         sockfd;
pid_t       pid;
struct      sockaddr_in  seraddr;
struct      sockaddr_in  locaddr;
struct      winsize      wsize;  
struct      cmd_t        cmd;


int main(int argc, char *argv[])
{
    char            slave_name[20];
    pthread_t       ptos, stop;
    pthread_attr_t  attr;
    int ret, ch, on = 1;
    wsize.ws_row    = 24;
    wsize.ws_col    = 80;

    char ser_ip[256] = "";  //  256 characters long, enough to store the complete domain name
    char ser_port[8] = "";
    char loc_ip[20]  = "";
    char loc_port[8] = "";

    unsigned char iserip_seted    = 0;
    unsigned char iserport_seted  = 0;
    unsigned char islocip_seted   = 0;
    unsigned char islocport_seted = 0;
    
	while((ch = getopt(argc, argv, OPTIONS)) != -1)
	{
		switch(ch)
		{
			case 'l': 
                strncpy(loc_ip, optarg, sizeof(loc_ip));
                islocip_seted =1;
                break;
			case 't': 
                strncpy(loc_port, optarg, sizeof(loc_port));
                islocport_seted = 1;
                break;
			case 's': 
                strncpy(ser_ip, optarg, sizeof(ser_ip));
                iserip_seted = 1;
                break;
			case 'p': 
                strncpy(ser_port, optarg, sizeof(ser_port));
                iserport_seted = 1;
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
    
    signal(SIGHUP,  SIG_IGN);
    signal(SIGINT,  sig_exit);
    signal(SIGQUIT, sig_exit);
    signal(SIGTERM, sig_exit);
    signal(SIGCHLD, sig_child);
    signal(SIGPIPE, sig_pipe);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    

    memset((char *)&seraddr, 0, sizeof(seraddr));
    memset((char *)&locaddr, 0, sizeof(locaddr));
    seraddr.sin_family = AF_INET;
    locaddr.sin_family = AF_INET;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
        perror("socket"),exit(1);
    
    if(!(iserip_seted && iserport_seted))
    {
        usage(basename(argv[0]));
        exit(1);
    }
    
    inet_pton(AF_INET, ser_ip, &seraddr.sin_addr);
    seraddr.sin_port = htons((unsigned short)atoi(ser_port));

    if(islocip_seted)
        inet_pton(AF_INET, loc_ip, &locaddr.sin_addr);
    if(islocport_seted)
        locaddr.sin_port = htons((unsigned short)atoi(loc_port));

    if(islocip_seted || islocport_seted)
    {
        ret = bind(sockfd, (struct sockaddr *)&locaddr, sizeof(locaddr));
        if(ret != 0)
        {  
            perror("bind");
            usage(basename(argv[0]));
            exit(1);
        }
    }

    setsockopt(sockfd, SOL_SOCKET,  SO_REUSEADDR, &on, sizeof(on));
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,  &on, sizeof(on));

    ret = connect(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr));
    if(ret != 0)
    {
        perror("connect");
        exit(1);
    }

    pid = pty_fork(&ptyfd, slave_name, sizeof(slave_name), NULL, &wsize);
    
    if(pid < 0)
        perror("pty_fork"),exit(1);
    if(pid == 0)
    {
        struct passwd *pwd;
        pwd = getpwuid(geteuid());
        chdir(pwd->pw_dir);
        execl(pwd->pw_shell, basename(pwd->pw_shell), NULL);
        exit(-1);
    }
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&ptos, &attr, pty_to_sock, NULL);
    pthread_create(&stop, &attr, sock_to_pty, NULL);
    pthread_join(ptos, NULL);
    pthread_join(stop, NULL);

    while(1)
        pause(); 

    return 0;
}
