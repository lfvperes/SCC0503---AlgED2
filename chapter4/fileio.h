#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define PMODE 0755

#define DELIM_STR "|"
#define DELIM_CHR '|'

#define out_str(fd,s) write((fd),(s),strlen(s)); write((fd), DELIM_STR,1);

#define fld_to_recbuff(rb, fld) strcat(rb,fld); strcat(rb,DELIM_STR)

#define MAX_REC_SIZE 512

#ifndef GET_RF
#define GET_RF
short getRec(int fd, char recBuff[]);
short getFld(char field[], char recBuff[], short scanPos, short recLength);
#endif