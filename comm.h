#ifndef _COMM_H_
#define _COMM_H_ 

typedef struct cmd_t{
    short row;
    short col;
    char  flag;
    char  size;
    char  key[4];
} cmd_t;

#define    SET_KEY        0x01
#define    SET_WINDOW     0x02

#define    VERSION        "1.2.3"
#define    BUFSIZE        1024

#endif
