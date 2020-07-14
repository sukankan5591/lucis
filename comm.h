#ifndef _COMM_H_
#define _COMM_H_ 

typedef struct cmd_t{
    short row;
    short col;
    char  flag;
    char  size;   // effect length of key
    char  key[6]; // 6 byte enough to save any key
} cmd_t;

#define    SET_KEY        0x01
#define    SET_WINDOW     0x02

#define    VERSION        "1.2.5"
#define    BUFSIZE        1024

#endif
