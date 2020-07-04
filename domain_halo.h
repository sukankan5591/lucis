#ifndef _DOMAIN_HALO_H_
#define _DOMAIN_HALO_H_

typedef struct ipset_t {
    int   count;
    struct in_addr **array;
}ipset_t;


/*
   count: number of how many IP resolved, if parameter(name) is
          already a legal IP addr, count = 1;
   array: direct to a array of struct in_addr{}

typical examples:

{
count:3
array: --> [0] --> struct in_addr{}
}          [1] --> struct in_addr{}
           [2] --> struct in_addr{}

this function use malloc, need to pay attention to release memory 
to avoid memory leak when running the program multiple times 
*/

int  resolve_domain(const char *name, struct ipset_t *buf);
void resolve_free(struct ipset_t *buf);

#endif
