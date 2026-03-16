#include "fileio.h"
#include <stdlib.h>

char recBuff[MAX_REC_SIZE+1];
char *prompt[] = {
    "Enter last name -- or <CR> to exit: ",
    "                        First name: ",
    "                           Address: ",
    "                              City: ",
    "                             State: ",
    "                               ZIP: ",
    ""
};

int main() {
    char response[50];
    char filename[15];
    int fd, fieldCounter;
    short recLength;

    printf("Enter the name of the file you wish to create: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;
    
    if ((fd = creat(filename, PMODE)) < 0) {
        printf("file opening error --- program stopped\n");
        exit(1);
    }

    printf("\n\n%s", prompt[0]);
    if (fgets(response, sizeof(response), stdin) == NULL) {
        printf("Error reading %s.\n", prompt[0]);
        close(fd);
        exit(1);
    }
    response[strcspn(response, "\n")] = 0;

    while(strlen(response) > 0) {
        recBuff[0] = '\0';
        fld_to_recbuff(recBuff, response);
        for (fieldCounter = 1; *prompt[fieldCounter] != '\0'; fieldCounter++) {
            printf("%s", prompt[fieldCounter]);
            
            if (fgets(response, sizeof(response), stdin) == NULL) {
                printf("Error reading %s.\n", prompt[fieldCounter]);
                close(fd);
                exit(1);
            }
            response[strcspn(response, "\n")] = 0;

            fld_to_recbuff(recBuff, response);
        }

        /* write out record length and buffer contents */
        recLength = strlen(recBuff);
        write(fd, &recLength, sizeof(recLength));
        write(fd, recBuff, recLength);

        /* prepare for next entry */
        printf("\n\n%s", prompt[0]);
        
        if (fgets(response, sizeof(response), stdin) == NULL) {
            printf("Error reading %s.\n", prompt[fieldCounter]);
            close(fd);
            exit(1);
        }
        response[strcspn(response, "\n")] = 0;
    }

    close(fd);

    return 0;
}