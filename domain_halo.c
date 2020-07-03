#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "domain_halo.h"


int domain(const char *name, struct ipset_t *buf)
{
    char **pptr;
    struct hostent *hptr = NULL;
    struct in_addr **p   = NULL;
    int ret, n, i;

    ret = inet_aton(name, NULL);
    if(ret == 1)
    {
        buf->count = 1;
        p    = (struct in_addr **)malloc(sizeof(struct in_addr *) * buf->count);
        p[0] = (struct in_addr *)malloc(sizeof(struct in_addr));
        inet_aton(name, p[0]);
        buf->array = p;
    }
    else
    {
        hptr = gethostbyname(name);
        if(hptr == NULL)
        {
            buf->count = 0;
            buf->array = NULL;
        }
        else
        {
            pptr = hptr->h_addr_list;
            for(n=0; *pptr!=NULL; pptr++)
                n++;
            
            p = (struct in_addr **)malloc(sizeof(struct in_addr *) * n);

            for(pptr=hptr->h_addr_list,i=0; *pptr!=NULL; pptr++,i++)
            {
                p[i] = (struct in_addr *)malloc(sizeof(struct in_addr));
                memcpy(p[i], *pptr, sizeof(struct in_addr));
            }
            buf->count = n;
            buf->array = p;
        }
    }

    return buf->count;
}
