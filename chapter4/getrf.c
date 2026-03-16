#include "fileio.h"

short getRec(int fd, char recBuff[]) {
    short recLength;

    if (read(fd, &recLength, 2) == 0) /* get record length */
        return 0;                     /* return 0 if EOF */
    recLength = read(fd, recBuff, recLength); /* read record */
    return recLength;
}

short getFld(char field[], char recBuff[], short scanPos, short recLength) {
    short fPos = 0; /* position in "field" array */

    if (scanPos == recLength) /* if no more fields to read */
        return 0;             /* return scanPos of 0 */
    
    /* scanning loop */
    while (scanPos < recLength && (field[fPos++] = recBuff[scanPos++]) != DELIM_CHR);

    if (field[fPos-1] == DELIM_CHR) /* if last character is a field */
        field[--fPos] = '\0';       /* delimiter, replace with null */
    else
        field[fPos] = '\0';         /* otherwise ensure the field is null-terminated */

    return scanPos;
}