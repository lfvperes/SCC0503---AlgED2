/* program to open or create a fixed length record file for 
 * updating. Records may be added or changed. Records to be changed must 
 * be accessed by relative record number 
 */

#include "fileio.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define REC_LGTH 64
char *prompt[] = {
    "                         Last name: ",
    "                        First name: ",
    "                           Address: ",
    "                              City: ",
    "                             State: ",
    "                               ZIP: ",
    ""
};

static int fd;
static struct {
    short recCount;
    char fill[30];
} head;

static int menu();
static void askInfo(char recBuff[]);
static int askRRN();
static int readAndShow();
static int change();

int main() {

    int i, menuChoice, RRN, bytePos;
    char filename[15];
    // long lseek();
    char recBuff[MAX_REC_SIZE+1]; // buffer to hold a record
    
    printf("Enter the name of the file: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;
    if ((fd = open(filename, O_RDWR)) < 0) { // if open fails
        fd = creat(filename, PMODE);         // then creat
        head.recCount = 0;                   // initialize header
        write(fd, &head, sizeof(head));      // write header rec
    } else                                   // existing file opened -- read in header
        read(fd, &head, sizeof(head));
    
    // main program loop
    while((menuChoice = menu()) < 3) {
        switch(menuChoice) {
            case 1:           // add new record
                printf("Input the information for the new record--\n\n");
                askInfo(recBuff);
                bytePos = head.recCount * REC_LGTH + sizeof(head);
                lseek(fd, (long)bytePos, 0);
                write(fd, recBuff, REC_LGTH);
                head.recCount++;
                break;
            case 2:           // update existing record
                RRN = askRRN();
                // if RRN is too big, print error message
                if (RRN >= head.recCount) {
                    printf("Record number is too large");
                    printf("... Returning to menu");
                    break;
                }
                // otherwise seek to the record
                bytePos = RRN * REC_LGTH + sizeof(head);
                lseek(fd, (long)bytePos, 0);

                // display it and ask for changes
                readAndShow();
                if (change()) {
                    printf("\n\nInput the revised values:\n\n");
                    askInfo(recBuff);
                    lseek(fd, (long)bytePos, 0);
                    write(fd, recBuff, REC_LGTH);
                }
                break;
        }
    }

    // rewrite correct record count to header before leaving
    lseek(fd, 0L, 0);
    write(fd, &head, sizeof(head));
    close(fd);
    
    return 0;
}

/* local function to ask user for next operation.
 * returns numeric value of user response
 */
static int menu() {
    int choice;
    char response[10];

    printf("\n\n\n\n    File updating program\n");
    printf("\n\nYou may choose to:\n\n");
    printf("\t1. Add a record to the end of the file\n");
    printf("\t2. Retrieve a record for updating\n");
    printf("\t3. Leave the program\n\n");
    printf("Enter the number of your choice: ");
    fgets(response, sizeof(response), stdin);
    response[strcspn(response, "\n")] = 0;
    choice = atoi(response);
    
    return choice;
}

/* local function to accept input of name and address fields, 
 * writing them to the buffer passed as a parameter
 */
static void askInfo(char recBuff[]) {
    int fieldCount, i;
    char response[50];

    // clear the record buffer
    for (i = 0; i < REC_LGTH; recBuff[i++] = '\0');

    // get the fields
    for (i = 0; *prompt[i] != '\0'; i++) {
        printf("%s", prompt[i]);
        fgets(response, sizeof(response), stdin);
        response[strcspn(response, "\n")] = 0;
        fld_to_recbuff(recBuff, response);
    }

    return;
}

/* local function to ask for the relative record number of the 
 * record that is to be updated
 */
static int askRRN() {
    int RRN;
    char response[10];

    printf("\n\nInput the Relative Record Number of the Record that you want to update: ");
    fgets(response, sizeof(response), stdin);
    response[strcspn(response, "\n")] = 0;
    RRN = atoi(response);

    return RRN;
}

/* local function to read and display a record. This function does
 * not include a seek --- reading starts at the current position 
 * in the file
 */
static int readAndShow() {
    char recBuff[MAX_REC_SIZE + 1], field[MAX_REC_SIZE + 1];
    int scanPos, dataLgth;

    scanPos = 0;
    read(fd, recBuff, REC_LGTH);


    printf("\n\n\nExisting record contents\n");
    recBuff[REC_LGTH] = '\0';   // ensure that record ends with null
    dataLgth = strlen(recBuff);
    while ((scanPos = getFld(field, recBuff, scanPos, dataLgth)) > 0)
        printf("\t%s\n", field);
}

/* local function to ask user whether or not they want to change the
 * the record. returns 1 if the answer is yes, 0 otherwise
 */
static int change() {
    char response[10];

    printf("\n\nDo you want to change this record?\n");
    printf("    Answer Y or N, followed by Enter ==>");
    fgets(response, sizeof(response), stdin);
    response[strcspn(response, "\n")] = 0;
    // ucase(response, response); // Remove this line
    
    // Check for both uppercase and lowercase 'Y'
    return (response[0] == 'Y' || response[0] == 'y') ? 1 : 0;
}