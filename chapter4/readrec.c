#include "fileio.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main() {

    int fd, recCount, fldCount, scanPos;
    short recLength;
    char filename[15];
    char recBuff[MAX_REC_SIZE+1];
    char field[MAX_REC_SIZE+1];

    printf("Enter name of file to read: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;

    if ((fd = open(filename, O_RDONLY)) < 0) {
        printf("file opening error --- program stopped\n");
        exit(1);
    }

    recCount = 0;
    scanPos = 0;
    while ((recLength = getRec(fd, recBuff)) > 0) {
        printf("Record %d\n", ++recCount);
        fldCount = 0;
        while ((scanPos = getFld(field, recBuff, scanPos, recLength)) > 0)
            printf("\tfield %d: %s\n", ++fldCount, field);
    }

    close(fd);
    
    return 0;
}