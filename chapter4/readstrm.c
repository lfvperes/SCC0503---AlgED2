#include "fileio.h"
#include <stdlib.h>

int readField(int fd, char fieldContent[]);

int main() {
    int fd, fieldLength, fieldCount;
    char fieldContent[30];
    char filename[15];

    printf("Enter the name of the file to read: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;
    if ((fd = open(filename, O_RDONLY)) < 0) {
        printf("file opening error --- program stopped\n");
        exit(1);
    }

    /* main program loop --- calls readField() for as long as the function succeeds */
    fieldCount = 0;
    while((fieldLength = readField(fd, fieldContent)) > 0)
        printf("\tfield # %3d: %s\n", ++fieldCount, fieldContent);
    
    close(fd);    

    return 0;
}

int readField(int fd, char fieldContent[]) {
    int i = 0;
    char c;

    /* byte-by-byte reading */
    while(read(fd, &c, 1) > 0 && c != DELIM_CHR)
        fieldContent[i++] = c;
    fieldContent[i] = '\0'; /* append null to end string */

    return i;
}