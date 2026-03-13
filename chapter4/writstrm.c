#include "fileio.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    char first[30], last[30], address[30], city[20];
    char state[15], zip[9];
    char filename[15];
    int fd;

    printf("Enter the name of the file you wish to create: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;

    if ((fd = creat(filename, PMODE)) < 0) {
        printf("file opening error --- program stopped\n");
        exit(1);
    }

    printf("\n\nType in a last name (surname), or Ctrl+C to exit\n>>>");
    if (fgets(last, sizeof(last), stdin) == NULL) {
        printf("Error reading last name.\n");
        close(fd);
        exit(1);
    }
    last[strcspn(last, "\n")] = 0;

    while (strlen(last) > 0) {
        printf("\nFirst name:");
        if (fgets(first, sizeof(first), stdin) == NULL) {
            printf("Error reading first name.\n");
            close(fd);
            exit(1);
        }
        first[strcspn(first, "\n")] = 0;

        printf("    Address:");
        if (fgets(address, sizeof(address), stdin) == NULL) {
            printf("Error reading address.\n");
            close(fd);
            exit(1);
        }
        address[strcspn(address, "\n")] = 0;

        printf("    City:");
        if (fgets(city, sizeof(city), stdin) == NULL) {
            printf("Error reading city.\n");
            close(fd);
            exit(1);
        }
        city[strcspn(city, "\n")] = 0;

        printf("    State:");
        if (fgets(state, sizeof(state), stdin) == NULL) {
            printf("Error reading state.\n");
            close(fd);
            exit(1);
        }
        state[strcspn(state, "\n")] = 0;

        printf("    Zip:");
        if (fgets(zip, sizeof(zip), stdin) == NULL) {
            printf("Error reading zip.\n");
            close(fd);
            exit(1);
        }
        zip[strcspn(zip, "\n")] = 0;

        /* output the strings to the buffer and then to the file */
        out_str(fd, last);
        out_str(fd, first);
        out_str(fd, address);
        out_str(fd, city);
        out_str(fd, state);
        out_str(fd, zip);
        write(fd, "#", 1);

        /* prepare for next entry */
        printf("\n\nType in a last name (surname), or Ctrl+C to exit\n>>>");
        if (fgets(last, sizeof(last), stdin) == NULL) {
            printf("Error reading last name.\n");
            close(fd);
            exit(1);
        }
        last[strcspn(last, "\n")] = 0;
    }
    /* close the file before leaving */
    close(fd);
    
    return 0;
}