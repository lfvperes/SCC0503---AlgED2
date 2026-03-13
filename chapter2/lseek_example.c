#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    int fd;
    char c;

    fd = open("test.txt", O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    read(fd, &c, 1);
    printf("First byte: %c\n", c);       /* prints 'H' from "Hello, World!" */

    lseek(fd, 7, SEEK_SET);              /* jump to position 7 */

    read(fd, &c, 1);
    printf("Byte at position 7: %c\n", c); /* prints 'W' */

    close(fd);
    return 0;
}