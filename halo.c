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
#include "env_halo.h"
#include "domain_halo.h"

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
    struct ipset_t  server_buf;
    int ret = -1, ch, on = 1;
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
                islocip_seted = inet_aton(loc_ip, NULL);;
                break;
			case 't': 
                strncpy(loc_port, optarg, sizeof(loc_port));
                if ((atoi(loc_port) > 0) && (atoi(loc_port) <= 65535))
                    islocport_seted = 1;
                else
                    islocport_seted = 0;
                break;
			case 's': 
                strncpy(ser_ip, optarg, sizeof(ser_ip));
                bzero(&server_buf, sizeof(server_buf));
                iserip_seted = resolve_domain(ser_ip, &server_buf);
                break;
			case 'p': 
                strncpy(ser_port, optarg, sizeof(ser_port));
                if ((atoi(ser_port) > 0) && (atoi(ser_port) <= 65535))
                    iserport_seted = 1;
                else
                    iserport_seted = 0;
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
    
    if(!(iserip_seted && iserport_seted))
    {
        usage(basename(argv[0]));
        exit(1);
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

    for(int i=0; i < server_buf.count; i++)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1)
            perror("socket"),exit(1);
        
        if(islocip_seted)
            inet_pton(AF_INET, loc_ip, &locaddr.sin_addr);
        if(islocport_seted)
            locaddr.sin_port = htons((unsigned short)atoi(loc_port));
        if(islocip_seted || islocport_seted)
        {
            ret = bind(sockfd, (struct sockaddr *)&locaddr, sizeof(locaddr));
            if(ret != 0)
            {  
                close(sockfd);
                perror("bind");
                usage(basename(argv[0]));
                exit(1);
            }
        }
        setsockopt(sockfd, SOL_SOCKET,  SO_REUSEADDR, &on, sizeof(on));
        setsockopt(sockfd, SOL_SOCKET,  SO_KEEPALIVE, &on, sizeof(on));
        setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,  &on, sizeof(on));

        seraddr.sin_addr.s_addr = ((struct in_addr *)(server_buf.array[i]))->s_addr;
        seraddr.sin_port        = htons((unsigned short)atoi(ser_port));
        
        ret = connect(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr));
        if(ret != 0)
        {
            close(sockfd);
            continue;
        }
        else
            break;
    }
    resolve_free(server_buf);

    if(ret == -1)
    {
        printf("Can not Connect To Server\n");
        close(sockfd);
        exit(1);
    }
    
    pid = pty_fork(&ptyfd, slave_name, sizeof(slave_name), NULL, &wsize);
    
    if(pid < 0)
        perror("pty_fork"),exit(1);
    if(pid == 0)
    {
        struct passwd *pwd;
        pwd = getpwuid(geteuid());
        handle_env(pwd);
        chdir(pwd->pw_dir);
        char *argv[] = {basename(pwd->pw_shell), NULL};
        execve(pwd->pw_shell, argv, halo_env);
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
